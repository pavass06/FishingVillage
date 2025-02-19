# FisherMan Class

The FisherMan class represents a household in the simulation that works as a fisherman. This class is derived from the base Household class (which is abstract), and it adds specific behavior and properties for the fishing village.

## Employment
- **Initial Employment:** 90% of fishermen are initially employed; 10% are unemployed.
- **Persistent Employment:** Once employed, a FisherMan remains employed until death. Unemployed fishermen actively seek jobs.

## Job Attributes
- **Job Sector:** Fixed to `"fishing"`.
- **Education Level, Experience Level, and Job Preference:** All are fixed at 1 for this simple model.

## Wage
- **Determination:** The daily wage is determined by the JobMarket. The final wage is set as 1.5 times the clearing wage provided by the JobMarket.

## Consumption
- **Fish Orders:** FisherMen submit orders to the FishingMarket to purchase fish, provided they have sufficient funds.

This design ensures that employment is persistent and only those without a job will seek employment each cycle.
