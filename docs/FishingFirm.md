# FishingFirm Class

The FishingFirm class represents a firm in the fishing village economy. It is responsible for producing fish and posting job vacancies.

## Production
- **Initial Stock:** Each firm starts with 50 fish.
- **Number of Employees:** Each firm employs 18 workers.
- **Sales Efficiency (α):** 2.0 – meaning each employee can sell 2 fish per day.
- **Goods Supply** is Calculated as :
  ```
  goodsSupply = min(stock, 2 × numberOfEmployees)
  ``` 

## Job Posting
- **Job Post Multiplier (δ):** 0.05  
- **Vacancies per firm** are calculated as:  
  ```
  vacancies = ceil(numberOfEmployees × δ)
  ```
  For 18 employees, this yields 1 vacancy per day.
- **Requirements:** Job posting requirements (education, experience, attractiveness) are all fixed at 1.

## Pricing and Revenue
- **Offered Price:** Each firm’s fish price is drawn from a normal distribution, N(6, 0.50) (initially set to 6 pounds per fish).
- **Revenue:** Calculated as:
  ```
  revenue = goodsSupply × offeredPrice
  ``` 
- **Business Expense:** Solely the wage cost:
  ```
  wageExpense = numberOfEmployees × (clearing wage from JobMarket)
  ``` 
- **Profit:** Revenue minus wage expense.
- **Investment:** A fraction of profit is reinvested to increase the firm's stock.

This class defines how firms produce fish, hire workers, and manage their finances in the simulation.



  
