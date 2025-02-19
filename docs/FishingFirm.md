# FishingFirm Class Documentation

The `FishingFirm` class models a firm in our fishing village economy. Each firm is responsible for producing fish, posting job vacancies, and setting the price for its fish. The parameters for each firm are initialized using statistical distributions to simulate natural variability.

## Parameter Initialization and Distributions

- **Funds:**  
  - Initialized using a normal distribution with a **mean of 100** and a **standard deviation of 20**.  
    This simulates differences in financial resources among firms.

- **Stock:**  
  - The initial stock of fish is drawn from a normal distribution with a **mean of 50** and a **standard deviation of 10**.  
    This represents the production capacity or inventory variability.

- **Employees:**  
  - Each firm employs **18 workers** (fixed for this simulation).

- **Sales Efficiency (α):**  
  - Set to **2.0**, meaning each employee can sell 2 fish per day on average.

- **Job Post Multiplier (δ):**  
  - Set to **0.05**.  
  - Vacancies are calculated as the ceiling of (`numberOfEmployees × δ`). With 18 employees, this yields:
    ```
    vacancies = ceil(18 × 0.05) = 1
    ```
    Thus, each firm posts at least 1 vacancy per day.

- **Offered Price:**  
  - Drawn from a normal distribution with a **mean of 6** and a **standard deviation of 0.5**.  
    This variability simulates differences in pricing strategies.

## Financial Metrics

- **Revenue:**  
  - Calculated as:
    ```
    revenue = min(stock, 2 × numberOfEmployees) × offeredPrice
    ```
- **Business Expense:**  
  - Determined by the wage cost:  
    ```
    wageExpense = numberOfEmployees × (clearing wage from JobMarket)
    ```
- **Profit:**  
  - Profit is revenue minus wage expense.
- **Investment:**  
  - A fraction of profit is reinvested to increase the firm's stock.

This structured approach with distributions gives each firm distinct characteristics, enhancing the realism of the simulation.
