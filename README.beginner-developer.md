# Tlf ham radio contest logger

## Summary

This is some collection to help recently joined coders to start with contributing TLF.
You can try the hard way. Then you just do not need to read any further.
Beleive me, at least for me, added unnecessary frustration not knowing any such collection.
You lean back, feeling (almost) on top of the World -- just to have to
learn somewhat later when asking for merge your work, that you are far from done...

In spite of every effort, this doc will (and can) never become complete.
Any comments, critisisms, suggestions, improvement requests are _ W E L C O M E .

To begin with, each project using any computer language, ususally is built upon
* partly some project conventions
* partly some personal tastes.

Individual tastes are no problem for one-man projects.
However, for bigger and long-lasting projects, individual tastes obviously
do not help readibility or maintenability.
My personal note, project conventions are also mainly built up of
personal taste of somebody in the project much before you joined ;-) ...

The following is the list we could collect so far.
This list is only numbered for easy reference.
This list is not considered to be sorted upon importance,
nor do I think there could be any such importance.
Please never re-number this list, that is, only add new points at end of file.
In case anybody feels in absolute need of picking some points upon importance,
please simply use a list in the header immediately below this introduction, like:

**`important by HA5SE: 	1, 2, 3		// sample`**



## Currently documented conventions


1)  TLF is a combination of old code mixed with many new updates.
    Old code is as-is, you should never count on conventions in old
    code would still continue to be considered as current.

    Recent code often uses different conventions.

    There is the possibility to find old, outdated conventions in old code before 200???? or V1???
    It is better to use examples in more recent code.

    You can check the age of any line or block using

    **`git blame ...`**


2)  No need to update **`  Changelog  `** . It only will be updated once for all changes 
    per each single new release by the maintainer. [thanks to HA5CQZ]


3)  In turn, it is your responsibility to update the manual page  **`  tlf.1.in  `**  if your contribution
    brings in visible changes to end-users. Such changes include e.g. new config keywords.


4)  Do not include your callsign or reason or similar in your modification. [thanks to HA5CQZ]
    No need to insert anything like

    **`// 2021-02-01 HA5SE  implementing TUNE_LNG=`**

    Instead of comments like that, rather count on

    **`git blame`**


5)  When defining global variables we have two choices: [thanks to DL1JBE]

    * Variables which are only part of an module should be defined in the module itself
    and made known to other modules by declaring them extern in the modules header file
    * Alternatively if the variable is not clearly related to a functional module we have
    concentrated them in one place - at the moment in  **`  main.c  `**  (with  **`  test/data.c  `**  a
    shadow copy for testing purposes). The variable should then be declared in  **`  globalvars.h  `** .
