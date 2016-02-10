mtd-rw
======

Sets the `MTD_WRITEABLE` flag on all MTD partitions that are
marked readonly. When unloading, read-only partitions will be
restored. Usage:
````
# insmod mtd-rw.ko i_want_a_brick=1
````

