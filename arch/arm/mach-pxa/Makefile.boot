<<<<<<< HEAD
__ZRELADDR     := $(shell /bin/bash -c 'printf "0x%08x" \
	$$[$(TEXT_OFFSET) + 0x80000000]')

   zreladdr-y  := $(__ZRELADDR)
=======
   zreladdr-y	+= 0xa0008000
>>>>>>> v3.4.6

