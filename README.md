# Aminer Citation Network Analysis Tool

The full data can be found at https://www.aminer.org/citation with the link to download the DBLP-Citation-network V12. A small subset can also be found in the data folder to run on with the name dblp_subset.v12.json. 

The contents of each of the folders in the repository are as follows:

- catch_tests: this contains the code to test our deliverables
- data: this is where the DBLP json data should go
- dataset: the code here is used for graph testing purposes
- graph: here is where the code for our graph implementations and algorithms are located
- lib: this where simdjson, the third party library we used to parse jsons (https://github.com/simdjson/simdjson), is located
- parsing: this is where the full code for parsing the entirety of the DBLP data is found
- src: this is where the code for what the user runs is found
- storage: the code for our data storage is located here (as well as some previous iterations of the current, v2 BTree database)





A functional code-base. Your code must either work on the default docker container or with special arrangements with your mentor in a system that you agreed on. It will be tested for reproducibility of your original results and it’s capacity to run on datasets of our choosing that exactly match your proposed formatting. Your code will be graded based on the following metrics:

    Code Execution – How easy is it to run your code? For full credit, your code should be runnable using simple command line arguments, which include the ability to alter or adjust the input data or output location.

    Code Efficiency – Does your code match your target Big O efficiencies? For full credit, your code should have no obvious inefficiency in implementation and be capable of running to completion on your proposed dataset using reasonable hardware resources.

    Code Organization – Is your code human-readable? For full credit, all your variables, functions, and classes should be named appropriately and organized comments should detail the input, output, and intended behavior of major code blocks. Additionally, your final submission should be devoid of unnecessary or obsolete code.

    Code Completion – Have you completed all your algorithms? For full credit, your code must be able to run all the proposed algorithms on the full dataset and have tests proving that the algorithms worked.

A descriptive README. In addition to the code itself, you must include a human-readable README.md which describes:

    Github Organization – You should describe the physical location of all major files and deliverables (code, tests, data, the written report, the presentation video, etc…)

    Running Instructions – You should provide full instructions on how to build and run your executable, including how to define the input data and output location for each method. You should also have instructions on how to build and run your test suite, including a general description on what tests you have created. It is in your best interest to make the instructions (and the running of your executables and tests) as simple and straightforward as possible.

A written report. In addition to your code, your Github repository must contain a results.md file which describes:

    The output and correctness of each algorithm – You should summarize, visualize, or highlight some part of the full-scale run of each algorithm. Additionally, the report should briefly describe what tests you performed to confirm that each algorithm was working as intended.

    The answer to your leading question – You should direct address your proposed leading question. How did you answer this question? What did you discover? If your project was ultimately unsuccessful, give a brief reflection about what worked and what you would do differently as a team.

A final presentation. In addition to your project write-up, you should submit a short video (10 minutes or less) describing your project. Your presentation should include slides or other visual aids and include the following content:

    Your Goals (Suggested time: 1-2 minutes) The presentation should begin with a summary of your proposed goals and a short statement about what you successfully accomplished and, if necessary, what you were ultimately unable to complete.

    Tip: Think of this as ‘setting the stage’ for your presentation, letting the viewer know what you will be discussing for the rest of the talk.

    Your Development (Suggested time: 2-3 minutes) The presentation should include a high level overview of the work you put into the presentation. This is not meant to be a line by line recounting of your code but a highlight reel of the various design decisions you made and the challenges you encountered – and hopefully overcame – while working on the project.

    If you were unable to complete one of your goals, this is the best opportunity to explain what you did that didn’t work out, how you tried to address the problem, and what you might do in the future if you were tasked to do this or a similar project again.

    Tip: If you are struggling to identify content here, ask yourself questions like: “How did we get the data we wanted?”, “How did we choose our implementation strategy for an algorithm?”, “How did we ultimately test our code to ensure that it is working?”

    Your Conclusions (Suggested time: 3-5 minutes) The presentation should end by answering the ‘leading question’ you were hoping to solve. This may include details such as the final or full-scale input dataset you used and the output of each of your algorithms but ambitious teams should focus on how these results led you to discover something interesting involving your real-world dataset. For example, a traversal algorithm on OpenFlights data may be used to identify the shortest path between two airports that your team would like to visit.

In addition to quantitative results, your conclusions should also end with some individual thoughts you had about the project. What did you learn, what did you like or didn’t like, and what would you explore or implement next if given more time?

To submit your final project video, you may either include it on Github or include a direct link to the video on your team Github. Videos can be hosted through Zoom cloud recordings, Youtube, Google drive, etc…
