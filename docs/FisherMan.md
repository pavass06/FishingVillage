# FisherMan Class Documentation

The `FisherMan` class represents an agent (or household) in the fishing village economy. Each FisherMan provides labor and consumes fish. Their demographic characteristics are drawn from statistical distributions to model natural variability.

## Parameter Initialization and Distributions

- **Age:**  
  - The initial age of a FisherMan is drawn from a normal distribution N(30, 10).  
    This reflects the spread in the working-age population.

- **Lifetime:**  
  - Drawn from a normal distribution N(60, 5), representing that agents are relatively young and have shorter lifespans.
  
- **Funds:**  
  - All FisherMen start with **0 funds**. This means there is no initial inheritance, and every agent begins on equal financial footing.

- **Employment Status:**  
  - **Initial Condition:** 90% of FisherMen are employed, and 10% are unemployed.
  - **Persistent Employment:** Once employed, a FisherMan remains employed until his death. Only those who are unemployed will actively seek a job each cycle.

- **Job Attributes:**  
  - **Job Sector:** Fixed to `"fishing"`.
  - **Education Level, Experience Level, and Job Preference:** All are fixed at **1** in this simple model.

- **Wage:**  
  - The daily wage for an employed FisherMan is determined by the JobMarket and is set as **1.5 times the clearing wage**.

## Consumption Behavior

- **Fish Orders:**  
  - FisherMen submit fish orders to the FishingMarket if they have sufficient funds.
  - The order quantity is randomly determined (between 1 and 3 fish) using a uniform distribution.
  - The maximum price a FisherMan is willing to pay is drawn from a normal distribution with a **mean of 5** and a **standard deviation of 0.8**.

This initialization using distributions ensures that while all FisherMen start with equal funds, their age and lifetime vary naturally, affecting their overall behavior in the simulation.
