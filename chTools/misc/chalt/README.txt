Work only in direct access mode, must develop daemonized access mode.
In "main.c" between l.19 and l.20 write 1 at address 0x120 to enable poweroff host+chantilly and not host only as currently.
Make the fork "chsleep".
