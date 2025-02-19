# FishingMarket Class

The FishingMarket is where fish are bought and sold. It aggregates fish offerings from FishingFirms and fish orders from FisherMen.

## Functionality
- **Consumer Orders:**  
  - Each FisherMan submits an order specifying a quantity (between 1 and 3) and a maximum price drawn from a normal distribution, N(5, 0.80).
- **Matching:**  
  - Orders are matched sequentially with fish offerings.
  - A match occurs if the consumer’s perceived maximum price is at least the firm’s offered price.
- **Clearing Fish Price Adjustment:**  
  After matching:
  - If demand exceeds supply, the new fish price is multiplied by a factor drawn from N(1.025, 0.005).
  - If supply exceeds demand, it is multiplied by a factor drawn from N(0.975, 0.005).
- **Inflation:**  
  - Inflation is computed as the percentage change in the fish price from the previous day.

This market mechanism sets the price of fish based on supply and demand dynamics.