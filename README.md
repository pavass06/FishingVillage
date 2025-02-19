# Fishing Village Economy Simulation

This project simulates a simple economy representing a fishing village, where the only product is fish. The model is designed to capture the essential dynamics of a small-scale economy, including population structure, firm production, labor market behavior, and market clearing for goods.

## Global Simulation Parameters

- **Population:**  
  - 100 FisherMen (agents), with 90 initially employed and 10 unemployed.
  - Ages are drawn from a normal distribution N(30, 10) to represent variability in the working-age population.
  - Lifetimes are drawn from a normal distribution N(60, 5), reflecting that agents are relatively young and may die early.
  - A birth rate of 0.1 per day is assumed, meaning on average one new FisherMan is born every 10 days.

- **Firms:**  
  - There are 5 FishingFirms.
  - Each firm employs 18 workers. With 18 employees per firm and a job post multiplier (δ) of 0.05, each firm posts a vacancy computed as `ceil(18 × 0.05)` (i.e., at least 1 vacancy per day), which helps keep unemployment around 10% initially.
  
- **Markets:**  
  - The **JobMarket** aggregates job postings from firms and applications from unemployed FisherMen. Once employed, a FisherMan remains employed until death.
  - The **FishingMarket** aggregates fish offerings from firms and orders from FisherMen. Prices are adjusted based on supply and demand using Gaussian factors.

For more detailed explanations on how each component works, see the subsequent documents:
- [Agent.md](agent.md)
- [FisherMan.md](fisherman.md)
- [FishingFirm.md](fishingfirm.md)
- [JobMarket.md](jobMarket.md)
- [FishingMarket.md](fishingMarket.md)
- [World.md](world.md)


