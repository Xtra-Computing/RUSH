# RUSH
## Introduction

In the ever-evolving landscape of e-commerce, the real-time detection of fraudulent activities is crucial for platforms like Grab, one of Southeast Asia's largest tech enterprises, known for its diverse services including digital payments and food delivery. This paper proposes RUSH (Real-time bUrst SubgrapH discovery framework), a pioneering framework tailored for real-time fraud detection within dynamic graphs. RUSH addresses the phenomenon of `burst subgraphs,' characterized by rapid increases in subgraph density within short timeframes—a scenario traditional fraud detection methods, which analyze static graphs, and existing incremental frameworks struggle to efficiently manage due to their inability to handle sudden surges in data. By focusing on both the density and the rate of change of subgraphs, RUSH identifies crucial indicators of fraud. Utilizing a sophisticated incremental update mechanism, RUSH processes large-scale graphs with high efficiency, catering to the demands of the fast-paced e-commerce industry. Furthermore, RUSH is equipped with user-friendly APIs, facilitating the customization and integration of specific fraud detection metrics. Through extensive experimental evaluations on real-world datasets, we demonstrate RUSH's efficiency and effectiveness in fraud detection. Through case studies, we illustrate how RUSH can effectively detect fraud communities within various Grab business scenarios and identify wash trading in NFT networks.

This is the source code for the paper: 'RUSH: Real-time Burst Subgraph Detection in Dynamic Graphs'.

## Compile

Our framework requires c++18 and GCC 8.x (or later). One can compile the code by executing the following commands.

```shell
git clone https://github.com/Alexcyh7/RUSH.git
cd RUSH
mkdir build
cd build
cmake ..
make
```

## Execute

After a successful compilation, the binary file is created under the `build/` directory. One can execute RUSH using the following command.

```shell
./rush <static_file> <increment_file> <start_time> <time_slice_length> <half_life_ratio> <retirement_ratio> <output_directory>
```

where `<static_file>` is the data graph file, and `<increment_file>` is the increment data graph file. `<start_time>` is the start time for the incremental updating.

### Data Graph & Pattern

Each line in the data graph file represent an edge.

An edge is represented by `<vertex-id-1> <vertex-id-2> <graph_weight> <timestamp>`.

For example, a 4-clique data graph with weight for each edge as 0.5 (with fake timestamp from 1 to 6) can be represented by

```
0 1 0.5 1
0 2 0.5 2
0 3 0.5 3
1 2 0.5 4
1 3 0.5 4
2 3 0.5 6
```

## Datasets and Querysets

We provide some test dataset and query example in dataset folder. The complete NFT graph datasets used in our paper can be downloaded [here](https://livegraphlab.github.io/)

## Example
On the 'build' directory, one can run the following script to conduct the burst subgraph detection on the example bayc graph.
```shell
mkdir bayc_potential_fraud
./rush ../dataset/static.txt ../dataset/increment.txt 1650364600 3600 1 16 ./bayc_potential_fraud
```

## Algorithm Scratch
We provde a scratch code to show how a user can use RUSH library to design a burst detection system.
The BuildWeightMap function takes the type of weight function as input. Users can specify WeightFunctionType::User and implement their own weight function.
```c
BurstGraph<false> graph;
graph.LoadGraphFromFile(input_graph_file);
graph.BuildWeightMap(WeightFunctionType::Degree);
graph.GeneratePeelingSequence();
graph.FindDenseSubgraph();
```
