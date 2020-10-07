Part 1:
./diskinfo <disk_name>

Part 2:
./disklist <disk_name>

Part 3:
./diskget <disk_name> /<file_name>

//Note: No sub-directory function implemented

Part 4:
./diskput <disk_name> <input_file> /<file_name>

/* Note: <file_name> in the program is hardcoded as <input_file>,
so actually the fourth argument is not used at all.
diskput only work on root directory */
