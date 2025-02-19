# Agent

This document explains the common parameters and properties used for all agents in the simulation.

## Common Parameters (for all Agents)
- **ID:** A unique identifier for each agent.
- **Funds:** The monetary resources available to the agent.
- **Status:** The current state (active or inactive/dead).
- **Age:** The number of cycles (days) since the agent’s creation.
- **Lifetime:** The maximum age (in days) the agent can reach before death.

## Household-Specific Parameters
- **Job Demand:** An indicator showing whether the household is actively seeking a job.
- **Goods Demand:** The daily quantity of fish the household wishes to purchase (randomly between 1 and 3 units).

These parameters form the base upon which specific agent types (such as FisherMen) build additional functionality.
