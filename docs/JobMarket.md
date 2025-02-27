# JobMarket Class

The JobMarket aggregates job postings from firms and job applications from unemployed FisherMen, and it matches them based on fixed criteria.

## Functionality
- **Aggregation:** Collects job postings (from firms) and job applications (from unemployed FisherMen) in the `"fishing"` sector.
- **Matching:** 
  - Matches are done sequentially.
  - A match occurs if the posting’s sector matches the application’s sector.
  - In this simple model, requirements are fixed at 1, so matching is straightforward.
- **Clearing Wage Adjustment:**  
  - The starting wage is at 5 * 1,5 = 7,5
  - The wages evolve with the current price of fish which is impacted by inflation :
    ```
    clearingPrice = price of a current fish * mean of fisher order by person
    ``` 
    c.f.[FishingMarket.md](fishingMarket.md) for more
  

This class forms the basis for how employment is managed in the simulation.
