# World Class Documentation

The `World` class orchestrates the entire simulation. It manages the population of FisherMen, the set of FishingFirms, and the integration of the JobMarket and FishingMarket over a specified number of cycles (days).

## Key Responsibilities

- **Time Management:**  
  - Each cycle represents one day.
  - The simulation runs for a fixed number of cycles as specified by the simulation parameters.

- **Population Management:**  
  - **Agents:**  
    - The world starts with 100 FisherMen (90 employed, 10 unemployed).
    - FisherMen who exceed their lifetime are removed.
  - **Births:**  
    - New FisherMen are born with a probability of 0.1 per day (approximately 1 new FisherMan every 10 days).

- **Market Integration:**  
  - **JobMarket:**  
    - Firms post job vacancies (calculated as `ceil(numberOfEmployees × 0.05)` per firm).
    - Only unemployed FisherMen submit job applications.
    - Once employed, a FisherMan remains employed until death.
    - The clearing wage is adjusted using a Gaussian factor, and the final wage is 1.5 times this clearing wage.
  - **FishingMarket:**  
    - Firms submit fish offerings and FisherMen submit orders.
    - The fish price is adjusted based on supply and demand using Gaussian factors.
    - Inflation is calculated as the day-to-day percentage change in the fish price.

- **Economic Indicators:**  
  - **GDP:** Sum of the revenues from all FishingFirms.
  - **Unemployment Rate:**  
    ```
    Unemployment Rate = (Number of Unemployed FisherMen / Total FisherMen) × 100%
    ```
  - **Inflation:** Percentage change in the fish price from one day to the next.

This class provides the integration of all simulation components and allows for the measurement of key economic indicators.
