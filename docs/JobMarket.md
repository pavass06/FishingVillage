# JobMarket Class

The JobMarket aggregates job postings from firms and job applications from unemployed FisherMen, and it matches them based on fixed criteria.

## Functionality
- **Aggregation:** Collects job postings (from firms) and job applications (from unemployed FisherMen) in the `"fishing"` sector.
- **Matching:** 
  - Matches are done sequentially.
  - A match occurs if the posting’s sector matches the application’s sector.
  - In this simple model, requirements are fixed at 1, so matching is straightforward.
- **Clearing Wage Adjustment:**  
  After matching:
  - If demand exceeds supply, the clearing wage is adjusted by a factor drawn from N(1.025, 0.005).
  - If supply exceeds demand, the clearing wage is adjusted by a factor drawn from N(0.975, 0.005).
  - The final wage for FisherMen is 1.5 times the clearing wage.

This class forms the basis for how employment is managed in the simulation.
