Programmer: Abigail Ibarrola, Huy Duong (haduong)

Global Variables Usage:

minget:

1. prog:

2. partitions:

3. v_flag: 

4. sup_block: The struct superblock is global because we use many fields such
    as log_zone_size, blocksize inside superblock frequently in our functions,
    so it's easier access the superblock this way.

5. fs_start: The variable that keep track of the location of file system 
    information starting point. We reference this variable whenever we need
    to access a zone or obtain information for computed fields.

6. inode_start: 

7. comp_f: The struct comp_fields is global because many computed fields in
    the struct are useful when traversing zones specified within directory 
    path.
