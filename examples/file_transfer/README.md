# File Transfer

About
-----

The file transfer server will load the disk file to RAM, so DONOT transfer file with too big a size!

How to Start
------------

    $ touch /tmp/file.test && echo -e "hello\nkingpin" >> /tmp/file.test
    $ ./FileClient
    $ md5sum /tmp/file.test /tmp/file.test.copy

