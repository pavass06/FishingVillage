# README: Fishing Village Economic Model

## Abstract
It is a **fishing village** where the only product produced and sold is fish, with no difference in species or freshness. Moreover, **all households** share exactly the same characteristics: they have the same sex, the same level of experience and education, and no particular competencies. They also all begin with **zero funds**. Each individual (fisher) works from the time they enter the labor market until they die — no retirement and educational system is needed in this initial model. This first version aims to **thoroughly control** the impact of each parameter before introducing more complex scenarios or distinguishing products and population categories.

*(For details on how individual fishers are represented, see the **FisherMan** class. For how fishing firms operate, refer to **FishingFirm**.)*

---

## About the Population
In this simulation, the population is generated using a specific **age distribution** following a normal law \(N(30,20)\). Each individual’s **lifetime** follows another normal law \(N(60,5)\), when this lifetime is exceed, one die. By setting these parameters, we replicate a relatively young society where no one retires: they keep working until they die. Additionally, there is an **annual birth rate** of **2%**, approximating the UK’s historical rate, and the timing of these births follows a Poisson process to introduce randomness. Furthermore, fishers who **do not eat** for too many consecutive days (fixed at **5** in our simulation) die. This models the severe consequence of prolonged lack of food.

*(For how the simulation handles mortality, births, and population updates each cycle, see the **World** class.)*

---

## Among This Working Population, We Also Have Firms
A small fraction—roughly **8%**—of the population are owners of **fishing firms**. At the start, each firm employs enough fishers so that **unemployment** is around **10%**. These firms offer jobs in proportion to the population, with a daily number of posted jobs set to **10%** of the total population (the parameter **totalJobOffers**). Each firm holds an **initial stock** of fish determined by a normal distribution centered on \(\frac{\text{population}}{\text{numberFirms}}\), with a standard deviation of \(5\%\) of that mean. This initial stock ensures that, at the outset, the population can be fed at least once.

To produce fish each day, we assume the output of a firm is the **minimum** of its existing stock and \(\text{(numberOfEmployees)} \times \text{(employeeEfficiency)}\). We set **employeeEfficiency** to **2**, indicating that a typical fisher can catch two fish per day. Then, the next day’s stock is derived by investing a random fraction between 0 and 1 of the **daily profit** back into the stock, thus modeling that profit does not entirely convert into new fish but is partially reinvested.

*(For how stocks are managed and updated, as well as how production is computed, see the **FishingFirm** class.)*

---

## Managing Relationships Between These Two Agents (FishMarket & JobMarket)
Two principal classes control interactions between firms and fishers:

**(a) FishMarket**  
This class governs **fish prices** and handles purchasing decisions. In particular, we define two normal distributions: one for the **offered price** \(N(\text{offeredPriceMean}=5.1,\,0.5)\) and one for the **perceived price** \(N(\text{perceivedPriceMean}=5.0,\,0.8)\). A fisher buys a fish automatically if they did not eat the previous day (i.e., if they are starving) **and** they have enough money; otherwise, they only buy if \(\text{perceivedPrice} \ge \text{offeredPrice}\). Any fish that remains unsold is discarded.

After each day, we measure total **demand** and total **supply**. If demand is greater than supply, both **offeredPriceMean** and **perceivedPriceMean** are multiplied by a factor drawn from \(N(1.025,\,0.005)\). Conversely, if supply exceeds demand, they are multiplied by a factor from \(N(0.975,\,0.005)\). To quantify **inflation** on day \(n\), we use the relative change of **offeredPriceMean**:
\[
\text{Inflation}(n) \;=\; \frac{\text{offeredPriceMean}(n+1)\;-\;\text{offeredPriceMean}(n)}{\text{offeredPriceMean}(n)}.
\]

**(b) JobMarket**  
Since all fishers share the same qualifications, job matching is straightforward: any unemployed fisher accepts a job if it is available. However, each day, a fraction (**pQuit**, set to **10%**) of employed fishers quits, ensuring some turnover. Firms, for their part, post about **10%** of the population in new job vacancies, distributed among them as the **totalJobOffers** parameter. Meanwhile, the daily **wage** starts around **perceivedPriceMean** and evolves in accordance with price dynamics or “inflation.”  

*(For details on how daily price clearing is done, see the **FishMarket**. For how jobs are posted and matched, consult the **JobMarket** class.)*

---

## 5) The Class That Resets Data (World)
Finally, the **World** class orchestrates each daily cycle. It **resets** key variables, **clears** the fish price, monitors supply and demand, updates the population—applying births, deaths, and starvation—and then consolidates everything into economic indicators like **GDP**, **GDP per capita**, **unemployment**, and **inflation**. In essence, **World** unifies the other classes, ensuring that fishers, firms, and markets remain synchronized day by day.

---

## Enumeration of All Initial Parameters for the Simulation
For this economy, the following **initial parameters** are crucial:

- **totalCycles**: total number of simulated days (e.g., 300).  
- **totalFisherMen**: total number of fishers (e.g., 100).  
- **totalFirms**: total number of fishing firms (e.g., 8%).  
- **initialEmployed**: number of fishers employed at the start (e.g., 90%).  
- **initialWage**: baseline wage or fish price reference (e.g., 5.0).  
- **cycleScale**: number of days per year (e.g., 365).  
- **maxStarvingDays**: how many consecutive days without fish lead to death (e.g., 5).  
- **annualBirthRate**: percentage of population growth per year (e.g., 2%).  
- **offeredPriceMean**: mean offered price by firms at the start (e.g., 5.1).  
- **perceivedPriceMean**: mean price consumers perceive at the start (e.g., 5.0).  
- **pQuit**: daily probability that an employed fisher quits (e.g., 10%).  
- **totalJobOffers**: total daily job openings across firms (e.g., 10%).  
- **employeeEfficiency**: how many fish a single fisher catches per day (e.g., 2).  

By adjusting these parameters, one can explore a variety of scenarios within this simplified fishing-based economy.

## Documentation
For more detailed explanations on each component, refer to:
- [Agent.md](agent.md)
- [FisherMan.md](fisherman.md)
- [FishingFirm.md](fishingfirm.md)
- [JobMarket.md](jobMarket.md)
- [FishingMarket.md](fishingMarket.md)
- [World.md](world.md)
