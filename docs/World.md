# World Class

The World class orchestrates the entire simulation by managing agents, firms, markets, and population dynamics over time.

## Time
- **Cycle:** Each cycle represents one day.
- **Total Cycles:** The simulation runs for a specified number of days.

## Population Management
- **Agents:**  
  - FisherMen: The simulation starts with 100 FisherMen.
  - Initially, 90 FisherMen are employed, and 10 are unemployed.
- **Birth and Death:**  
  - Agents are removed when their age exceeds their lifetime.
  - New FisherMen are born with a probability of 0.1 per day (i.e., on average, 1 new FisherMan every 10 days).

## Market Integration
- **JobMarket:**  
  - Every day, firms post job vacancies and only unemployed FisherMen submit job applications.
  - Employed FisherMen remain employed until death.
  - The JobMarket clears, and the clearing wage is used to set FisherMen’s wages.
- **FishingMarket:**  
  - Firms submit fish offerings and FisherMen submit fish orders.
  - The market clears based on the matching of orders and offerings.
  - The fish price is adjusted based on Gaussian factors, and inflation is computed as the day-to-day percentage change.

## Economic Indicators
- **GDP:** The sum of revenues from all FishingFirms.
- **Unemployment Rate:** Calculated as:
  ```
  Unemployment Rate = (Number of Unemployed FisherMen / Total FisherMen) × 100
  ``` 
- **Inflation:** The percentage change in the fish price from one day to the next.

This class integrates all components of the simulation and calculates macroeconomic indicators for analysis.
