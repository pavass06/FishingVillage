# Fishing Village Economy Simulation

This project simulates a simple economy representing a fishing village, where the only product is fish. The model captures key dynamics of a small-scale economy including population structure, firm production, labor market behavior, and market clearing for goods.

## Global Simulation Parameters

- **Population:**  
  - 100 FisherMen (agents), with 90 initially employed and 10 unemployed.
  - Ages are drawn from a normal distribution *N(30, 10)*, representing variability in the working-age population.
  - Lifetimes are drawn from a normal distribution *N(60, 5)*, reflecting that agents are relatively young and may die early.
  - A birth rate of 0.1 per day is assumed (on average, one new FisherMan is born every 10 days).

- **Firms:**  
  - There are 5 FishingFirms.
  - Each firm employs 18 workers. With a job post multiplier (δ) of 0.05, each firm posts a vacancy computed as `ceil(18 × 0.05)` (i.e., at least 1 vacancy per day), keeping unemployment around 10% initially.

- **Markets:**  
  - The **JobMarket** aggregates job postings from firms and applications from unemployed FisherMen. Once employed, a FisherMan remains employed until death.
  - The **FishingMarket** aggregates fish offerings from firms and orders from FisherMen. Prices here are dynamically adjusted based on supply and demand.

## Key Modifications and Updates

### 1. Dynamic Price Distributions
- **Firm Offered Price Distribution:**  
  - Initially set as *N(6, 0.5)*.
- **Consumer Perceived Price Distribution:**  
  - Initially set as *N(5, 0.8)*.
- **Dynamic Updates:**  
  - After each simulation cycle, the underlying means for both distributions are updated.
  - An adjustment factor is drawn from a normal distribution:
    - If demand exceeds supply, the factor is drawn from *N(1.025, 0.005)*.
    - If supply exceeds demand, the factor is drawn from *N(0.975, 0.005)*.
  - These factors multiply the current means (e.g., 6.0 and 5.0) to evolve future offers and consumer perceptions in line with market conditions.

### 2. Revised Clearing Price Logic
- **Clearing Price as an Indicator:**  
  - In the FishingMarket class, the clearing price is now computed as the weighted average of all transaction prices (weighted by the deal volumes).  
  - This weighted average reflects the actual market price at which fish are sold during the cycle.
- **Separation of Concerns:**  
  - The clearing price serves solely as an indicator of actual transactions and does not directly update the price distributions.
  - The underlying price distributions are updated independently using the drawn adjustment factors as described above.

### 3. Added GDP Metrics
- **GDP per Capita:**  
  - Calculated for each cycle as the total GDP divided by the number of FisherMen.
- **GDP Growth:**  
  - Computed as the percentage change in GDP between successive cycles:
    \[
    \text{GDP Growth} = \frac{GDP_{n+1} - GDP_n}{GDP_n}
    \]
  - These metrics provide additional insight into the evolving economic performance of the simulation.

## Documentation
For more detailed explanations on each component, refer to:
- [Agent.md](agent.md)
- [FisherMan.md](fisherman.md)
- [FishingFirm.md](fishingfirm.md)
- [JobMarket.md](jobMarket.md)
- [FishingMarket.md](fishingMarket.md)
- [World.md](world.md)
