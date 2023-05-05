# Aminer Citation Network Analysis Tool

### Main components

The full data used in this project can be found at https://www.aminer.org/citation with the link to download the DBLP-Citation-network V12. 

The contents of each of the folders in the repository are as follows:

- catch_tests: this contains the code to test our deliverables
- data: this is where the DBLP json data should be placed
- dataset: the code here contained here is used for graph/algorithm testing purposes
- graph: here is where the code for our graph implementations and graph algorithms are located
- lib: this where simdjson, the third party library we used to parse jsons (https://github.com/simdjson/simdjson), is located
- parsing: this is where the full code for parse the input data (this is specifically geared towards the DBLP v12 dataset)
- src: this is where the code for what the user runs is found
- storage: the code for our data storage is located here (as well as some previous iterations of the current, v2 BTree database)

The main results can be found in the results.md markdown file also contained within this repository. The video presentation can be found at **(https://drive.google.com/file/d/1Rk9Xf_p1wZ18LRzYfDNgj7Bu1X6mjOBm/view?usp=sharing)**. The video slides are **(https://docs.google.com/presentation/d/1wDksKqL4wKqfdptBzfNlxBRiJCsjnVNhP7LKymr4JG4/edit?usp=sharing)**

### Build instructions

To run this code, clone this repository (git clone https://github.com/dayzhang/journal-graph.git) into the CS225 docker container and run the following:

```
mkdir build
cd build
cmake ..
```

After doing this, you can then run ```make``` to build all the code. At this point, there are a few things that can be run.

- ./parse [path to the dblp json relative to the build folder]
    - This reads in and parses the DBLP data in two passes to construct the database and graph binary files, which are deposited into the build folder under the names author_keys.db, author_values.db, paper_keys.db, paper_values.db, author_graph.bin, and journalgraph.bin. 
    - This function is relatively intensive, so an alternative to running this is to download these things pre-generated at the following link: https://drive.google.com/file/d/1xvQGafQpwJB5L4UMroDryvZWvToigL75/view?usp=share_link
    - This is an archive file, and its contents should be directly placed into the build folder for the other functions to read.
- ./db_interface [key db filename, default *_keys.db] [value db filename, default *_values.db] [type of database (test, paper, or author)] [whether or not to create a new db (0 for no, 1 for yes)] [read only setting [0 for no, 1 for yes]]
    - This provides an interface to browse and query the database from the B+ Tree structure stored in the .db files. It is generally recommended to be read only/not to create a new db when working with the key/value db files to prevent data corruption, but this can be ignored if you are just making a dummy database for testing (which the test type is suited for). 
    - The commands available in this are explained in the actual code.
    - Fun queries include "G. Carl Evans" (id 2109906170), "Brad Solomon" (id 2189947603), "Geoffrey Challen" (id 2231335109), "Michael Nowak" (id 2688443206), "Geoffrey L. Herman" (id 2148163125), and "Lawrence Angrave" (id 2645015366) in the author database. 
    - The main functionality in this is the insert/find features; the other features are relatively slower as they bypass the BTree structure (although they get faster after repeated usage with more things loaded to memory). 
- ./paper_game [paper graph binary (journalgraph.bin)] [paper key db file (paper_keys.db)] [paper values db file (paper_values.db)] [start paper id (try 1091 if you don't have a specific one)]
    - This provides an interface to browse and explore the paper database, navigating only via neighbors. 
    - The commands for this are also found within the CLI once the code is run.
- ./main [graph type (Journals or Authors)]
    - ./main is the main place the run algorithms on the Journals and Authors graphs
    - The jounal option automatically runs DFS on a source node.
    - The authors option prompts the user for Tarjans and Dijkstra's, and runs each based on user input accordingly
- ./run_tests
    - This runs the tests we created to test our deliverables.
    - If valgrind is used, this may take a bit. 