#  Fisherman Simulation

This project runs an agent-based simulation of a fishing economy. Follow the steps below to execute the simulation and analyze the results.

---


## Part 1. Copy the Input File

Copy an example input into the working directory:

```bash
cp ../example/input_test input
```


## Part 2: Run the Simulation

To run the simulation (assumed program is compiled with `make` in `src`):

```bash
./agent.exe input
```

To save the output to a log file:

```bash
./agent.exe input > screen
```

This logs all simulation output (e.g., console messages and debug info) into the file named screen.

## Part 3: Output Files

The simulation produces the following CSV files in the current directory:


- *firm_revenu.csv* : Revenue history for each firm (one column per firm, one row per cycle)
- *unemploymentHistory.csv* : Unemployment rate tracked daily
- *economicdatas.csv* :  Summary per cycle: GDP, inflation, unemployment, population, etc.

## Part 4: Visualizing Results

You can analyze and visualize the output with Python scripts.

```bash
cp ../Analysis/*.py .
```
and

```bash
python show.py
```

Alternatively, execute the scripts directly from ```../Analysis``` (make sure output CSV files are accessible).

## NOTES

- Requires **Python 3.x**
- Dependencies: `pandas`, `matplotlib`  
  You can install them with:

```bash
  pip install pandas matplotlib
```
input file syntax:

  key value   # optional comment

For example:
```
TotalCycles 1000  # number of simulation cycles
cycleScale 365    # days per year
```