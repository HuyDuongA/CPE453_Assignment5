Programmer: Abigail Ibarrola, Huy Duong (haduong)

Global Variables Usage:

minget:

1. prog: The char array prog holds the name of the running program. We use it
    to print out errors.

2. partitions: This flag indicates whether option partition is input by user.
    We use the flag to indicate which parsing needed to be done before getting
    to the file sytem information block.

3. subpartitions: This flag indicates whether option subpartition is input by 
    user. We use the flag to indicate which parsing needed to be done before
    getting to the file sytem information block.

4. v_flag: This flag indicates whether option verbosity is input by 
    user. We use the flag to indicate whether to print out information inside 
    the superblock.

5. sup_block: The struct superblock is global because we use many fields such
    as log_zone_size, blocksize inside superblock frequently in our functions,
    so it's easier access the superblock this way.

6. fs_start: The variable that keep track of the location of file system 
    information starting point. We reference this variable whenever we need
    to access a zone or obtain information for computed fields.

7. inode_start: The inode_start holds the starting address of the inode table
    in the file system information block. Because we call get_inode that uses
    inode_start many times, the code looks cleaner when there is less number
    of arguments in get_inode function.

8. comp_f: The struct comp_fields is global because many computed fields in
    the struct are useful when traversing zones specified within directory 
    path.
