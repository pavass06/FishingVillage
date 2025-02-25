# FishingMarket Class

The FishingMarket class is responsible for matching fish offerings from FishingFirms with orders from FisherMen. It simulates the market mechanism for fish, calculates a clearing price, and provides an indicator of the actual market transaction price.

## Functionality

- **Consumer Orders:**  
  - Each FisherMan submits an order specifying a quantity (between 1 and 3) and a maximum price drawn from a normal distribution *N(5, 0.80)*.

- **Firm Offerings:**  
  - Each FishingFirm provides a fish offering with an offered price drawn from a normal distribution *N(6, 0.5)*.

- **Matching Process:**  
  - Orders are matched sequentially with fish offerings.
  - A transaction occurs if the FisherMan's perceived maximum price meets or exceeds the firm's offered price.
  - The transaction price is taken as the firm's offered price.

- **Clearing Price Calculation:**  
  - The clearing price is computed as the weighted average of all transaction prices, where each transaction’s price is weighted by its volume.
  - This weighted average serves as an indicator of the actual market price for fish in that cycle.

- **Market Reset:**  
  - After each cycle, aggregate demand and supply are reset to prepare for the next cycle's matching process.

- **Dynamic Updates in the Simulation:**  
  - Although the clearing price reflects the actual transaction outcomes, the underlying price distributions (for firm offers and consumer perceptions) are updated separately.
  - In the simulation, after matching:
    - If demand exceeds supply, the current underlying price means are multiplied by a factor drawn from *N(1.025, 0.005)*.
    - If supply exceeds demand, they are multiplied by a factor drawn from *N(0.975, 0.005)*.
  - This update mechanism ensures that future price offers and perceived values evolve dynamically while the clearing price remains an independent indicator of market conditions.
