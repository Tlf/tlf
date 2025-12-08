#include <curl/curl.h>
#include <glib.h>
#include <time.h>

#include <cabrillo_utils.h>
#include <showscore.h> // get_total_score

// configurable parameters
bool onlinescore = false;
char *onlinescore_url = NULL;
#define DEFAULT_ONLINESCORE_URL "https://contestonlinescore.com/post/"
char *onlinescore_user = NULL;
char *onlinescore_pass = NULL;

// Real Time Contest specification for XML postings:
// The logger should send a posting every 2 minutes even if there are no changes in a
// contest log. No other rates can be accessible in logger settings.
const int onlinescore_interval = 120;

// static data initialized by onlinescore_init
CURL *curl = NULL;
cbr_field_t *cbr_contest;
cbr_field_t *cbr_operators;
cbr_field_t *cbr_power;
cbr_field_t *cbr_assisted;
cbr_field_t *cbr_transmitter;
cbr_field_t *cbr_ops;
cbr_field_t *cbr_bands;
cbr_field_t *cbr_mode;
cbr_field_t *cbr_overlay;

static size_t
receive_data(char *contents, size_t size, size_t nmemb, void *userp) {
    // ignore data received for now
    return size * nmemb;
}

void onlinescore_init() {
    if (! onlinescore)
	return;

    if (! iscontest) {
	onlinescore = false;
	return;
    }

    if (! onlinescore_url) {
	onlinescore_url = strdup(DEFAULT_ONLINESCORE_URL);
    }

    curl = curl_easy_init();
    if (! curl) {
	// something went wrong with initializing the library, disable online score submissions
	onlinescore = false;
	return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, onlinescore_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_data);
    //curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &chunk); // ignore data received for now

    /* headers */
    struct curl_slist *slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/xml");
    slist1 = curl_slist_append(slist1, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
    // we'll reuse the headers for each invocation, don't free list

    if (onlinescore_user) {
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
	curl_easy_setopt(curl, CURLOPT_USERNAME, onlinescore_user);
	if (onlinescore_pass) {
	    curl_easy_setopt(curl, CURLOPT_PASSWORD, onlinescore_pass);
	} else {
	    curl_easy_setopt(curl, CURLOPT_NETRC, (long)true);
	}
    }

    // cache some cabrillo field lookups
    cbr_contest = find_cabrillo_field("CONTEST");
    cbr_operators = find_cabrillo_field("OPERATORS");
    cbr_power = find_cabrillo_field("CATEGORY-POWER");
    cbr_assisted = find_cabrillo_field("CATEGORY-ASSISTED");
    cbr_transmitter = find_cabrillo_field("CATEGORY-TRANSMITTER");
    cbr_ops = find_cabrillo_field("CATEGORY-OPERATOR");
    cbr_bands = find_cabrillo_field("CATEGORY-BAND");
    cbr_mode = find_cabrillo_field("CATEGORY-MODE");
    cbr_overlay = find_cabrillo_field("CATEGORY-OVERLAY");
}

void send_onlinescore() {
    if (! onlinescore)
	return;

    /* build the XML document */
    GString *data = g_string_new(NULL);

    g_string_append(data, "<?xml version=\"1.0\"?>\n");
    g_string_append(data, "<dynamicresults>\n");
    g_string_append_printf(data, "<contest>%s</contest>\n",
			   cbr_contest->value ? cbr_contest->value : whichcontest);
    g_string_append(data, "<soft>TLF</soft>\n");
    g_string_append(data, "<version>" VERSION "</version>\n");
    g_string_append_printf(data, "<call>%s</call>\n", my.call);
    if (cbr_operators->value) g_string_append_printf(data, "<ops>%s</ops>\n",
		cbr_operators->value);
    // TODO: <qth>

    g_string_append(data, "<class");
    if (cbr_power->value) g_string_append_printf(data, " power=\"%s\"",
		cbr_power->value);
    if (cbr_assisted->value) g_string_append_printf(data, " assisted=\"%s\"",
		cbr_assisted->value);
    if (cbr_transmitter->value) g_string_append_printf(data, " transmitter=\"%s\"",
		cbr_transmitter->value);
    if (cbr_ops->value) g_string_append_printf(data, " ops=\"%s\"", cbr_ops->value);
    g_string_append_printf(data, " bands=\"%s\"",
			   cbr_bands->value ? cbr_bands->value : "ALL");
    g_string_append_printf(data, " mode=\"%s\"",
			   cbr_mode->value ? cbr_mode->value : "MIXED");
    g_string_append_printf(data, " overlay=\"%s\"></class>\n",
			   cbr_overlay->value ? cbr_overlay->value : "N/A");

    g_string_append(data, "<breakdown>\n");
    int qsos = 0;
    int country_mults = 0;
    int zone_mults = 0;
    for (int i = 0; i < NBANDS; i++) {
	GString *band_name = g_string_new(band[i]);
	g_strchug(band_name->str); // remove leading whitespace
	if (qsos_per_band[i] > 0) {
	    qsos += qsos_per_band[i];
	    // TODO: include mode="" in these lines??
	    g_string_append_printf(data, "<qso band=\"%s\">%d</qso>\n", band_name->str,
				   qsos_per_band[i]);
	}
	if (countryscore[i] > 0) {
	    country_mults += countryscore[i];
	    g_string_append_printf(data, "<mult band=\"%s\" type=\"country\">%d</mult>\n",
				   band_name->str, countryscore[i]);
	}
	if (zonescore[i] > 0) {
	    zone_mults += zonescore[i];
	    g_string_append_printf(data, "<mult band=\"%s\" type=\"zone\">%d</mult>\n",
				   band_name->str, zonescore[i]);
	}
	g_free(band_name);
    }
    g_string_append_printf(data, "<qso band=\"total\">%d</qso>\n", qsos);
    g_string_append_printf(data,
			   "<mult band=\"total\" type=\"country\">%d</mult>\n", country_mults);
    g_string_append_printf(data, "<mult band=\"total\" type=\"zone\">%d</mult>\n",
			   zone_mults);
    g_string_append(data, "</breakdown>\n");
    g_string_append_printf(data, "<score>%d</score>\n", get_total_score());

    time_t now = time(NULL);
    struct tm n;
    gmtime_r(&now, &n);
    g_string_append_printf(data,
			   "<timestamp>%04d-%02d-%02d %02d:%02d:%02d</timestamp>\n",
			   n.tm_year + 1900, n.tm_mon + 1, n.tm_mday, n.tm_hour, n.tm_min, n.tm_sec);

    g_string_append_printf(data, "</dynamicresults>\n");

    /* send the request */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->str);
    curl_easy_perform(curl);
    g_free(data);
}

void *onlinescore_process(void *ptr) {
    while (1) {
	send_onlinescore();
	sleep(onlinescore_interval);
    }

    // never returns, terminated by pthread_cancel()
}
