#ifndef MARKET_H
#define MARKET_H

#include <iostream>
#include <random>

class Market {
protected:
    double aggregateDemand;  // Total quantity of buy orders/jobs.
    double aggregateSupply;  // Total quantity of sell orders.
    double clearingPrice;    // Current market clearing price.
    
public:
    Market(double initClearingPrice)
      : aggregateDemand(0.0), aggregateSupply(0.0), clearingPrice(initClearingPrice) {}

    virtual ~Market() {}

    // Modified to accept a random engine.
    virtual void clearMarket(std::default_random_engine &generator) = 0;

    virtual void reset() {
        aggregateDemand = 0.0;
        aggregateSupply = 0.0;
    }

    virtual void print() const {
        std::cout << "Market | Demand: " << aggregateDemand 
                  << " | Supply: " << aggregateSupply 
                  << " | Clearing Price: " << clearingPrice << std::endl;
    }

    double getClearingPrice() const { return clearingPrice; }
    double getAggregateDemand() const { return aggregateDemand; }
    double getAggregateSupply() const { return aggregateSupply; }
};

#endif // MARKET_H
