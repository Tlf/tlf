int key_get() {
    return -1;
}

unsigned int __wrap_sleep(unsigned int seconds) {
    // no sleeping in tests
    return 0;
}

