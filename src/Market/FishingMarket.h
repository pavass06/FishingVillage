#ifndef FISHINGMARKET_H
#define FISHINGMARKET_H

#include "Market.h"
#include "FishingFirm.h"   // full definition of FishingFirm
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <memory>
#include <unordered_map>
#include <iostream>

// -----------------------------------------------------------------------------
// FishOffering: one supply offer from a firm
// -----------------------------------------------------------------------------
#ifndef FISH_OFFERING_DEFINED
#define FISH_OFFERING_DEFINED
struct FishOffering {
    int id;
    std::string productSector;
    double cost;
    double offeredPrice;
    double quantity;
    std::shared_ptr<FishingFirm> firm;
};
#endif

// -----------------------------------------------------------------------------
// FishOrder: one buy order from a fisherman
// -----------------------------------------------------------------------------
struct FishOrder {
    int id;
    std::string desiredSector;
    double quantity;       // always 1.0 in your simplified model
    double perceivedValue; // max price if not hungry
    bool   hungry;         // if true, bids availableFunds instead
    double availableFunds; // max price if hungry
};

// -----------------------------------------------------------------------------
// FishingMarket: double‐auction market for fish
// -----------------------------------------------------------------------------
class FishingMarket : public Market {
private:
    std::vector<FishOffering>           offerings;
    std::vector<FishOrder>              orders;
    double                              aggregateSupply   = 0.0;
    double                              aggregateDemand   = 0.0;
    double                              matchedVolume     = 0.0;
    std::unordered_map<int,double>      purchases;        // fish bought per fisher
    std::vector<double>                 clearingPrices;   // history

public:
    // Constructor: seed initial clearing price
    FishingMarket(double initialPrice = 5.0)
      : Market(initialPrice)
    {
        clearingPrices.push_back(initialPrice);
    }
    virtual ~FishingMarket() {}

    // --- Accessors ---
    double getClearingFishPrice() const                 { return clearingPrice; }
    double getAggregateSupply()   const                 { return aggregateSupply; }
    double getAggregateDemand()   const                 { return aggregateDemand; }
    double getMatchedVolume()     const                 { return matchedVolume; }
    const std::vector<double>& getClearingPriceHistory()const { return clearingPrices; }
    const std::unordered_map<int,double>& getPurchases()const   { return purchases; }

    // --- Buyer interface ---
    // Submit a buy order and bump aggregateDemand.
    void submitFishOrder(const FishOrder& order) {
        orders.push_back(order);
        aggregateDemand += order.quantity;
    }

    // --- Seller interface ---
    // Collect stock from each firm into 'offerings' and bump aggregateSupply.
    void refreshSupply(const std::vector<std::shared_ptr<FishingFirm>>& firms) {
        offerings.clear();
        aggregateSupply = 0.0;
        for (auto& firm : firms) {
            firm->updateStock();                    // update internal stock
            double stock = firm->getStock();        // retrieve available units

            FishOffering o;
            o.id            = firm->getID();
            o.productSector = "fishing";
            o.cost          = 0.0;
            o.offeredPrice  = firm->getPriceLevel();
            o.quantity      = stock;
            o.firm          = firm;

            offerings.push_back(o);
            aggregateSupply += stock;
        }
    }

    // --- Market clearing: continuous double‐auction ---
    virtual void clearMarket(std::default_random_engine& /*generator*/) override {
        // Reset per‐round stats
        purchases.clear();
        matchedVolume     = 0.0;
        double sumValue   = 0.0;
        double totalVol   = 0.0;

        // Sort buy orders by descending bid, sell offers by ascending ask
        std::sort(orders.begin(), orders.end(),
            [](auto &a, auto &b) {
                double pa = a.hungry ? a.availableFunds : a.perceivedValue;
                double pb = b.hungry ? b.availableFunds : b.perceivedValue;
                return pa > pb;
            });
        std::sort(offerings.begin(), offerings.end(),
            [](auto &a, auto &b) {
                return a.offeredPrice < b.offeredPrice;
            });

        // Match while best bid ≥ best ask
        size_t i = 0, j = 0;
        while (i < orders.size() && j < offerings.size()) {
            auto &buy  = orders[i];
            auto &sell = offerings[j];
            double bidPrice = buy.hungry ? buy.availableFunds : buy.perceivedValue;

            if (bidPrice >= sell.offeredPrice 
                && buy.quantity  > 0.0 
                && sell.quantity > 0.0)
            {
                // Trade the lesser of the two quantities (here usually =1)
                double q       = std::min(buy.quantity, sell.quantity);
                double txPrice = sell.offeredPrice;

                // Update running totals
                buy.quantity       -= q;
                sell.quantity      -= q;
                matchedVolume      += q;
                totalVol           += q;
                sumValue           += txPrice * q;
                purchases[buy.id]  += q;

                // Credit the selling firm
                if (sell.firm) {
                    sell.firm->addSale(txPrice, q);
                }

                if (buy.quantity  <= 0.0) ++i;
                if (sell.quantity <= 0.0) ++j;
            }
            else {
                break; // No further matches possible
            }
        }

        // Compute volume‐weighted clearing price
        if (totalVol > 0.0) {
            clearingPrice = sumValue / totalVol;
        }
        clearingPrices.push_back(clearingPrice);

        // Clean up for next round
        offerings.clear();
        orders.clear();
        aggregateSupply   = 0.0;
        aggregateDemand   = 0.0;
        matchedVolume     = 0.0;
    }

    // --- Reset entire market ---
    virtual void reset() override {
        Market::reset();
        offerings.clear();
        orders.clear();
        aggregateSupply   = 0.0;
        aggregateDemand   = 0.0;
        matchedVolume     = 0.0;
        clearingPrices.clear();
    }

    // Optionally override aggregate demand
    void setAggregateDemand(double d) {
        aggregateDemand = d;
    }

    // Debug print
    virtual void print() const override {
#if verbose == 1
        std::cout << "-----------\nFishing Market State:\n";
        Market::print();
        double totalFish = 0.0;
        for (auto &o : offerings) totalFish += o.quantity;
        std::cout << "Matched Volume: " << matchedVolume << "\n"
                  << "Total Offered : " << totalFish    << "\n"
                  << "Orders Count  : " << orders.size()<< "\n";
#endif
    }
};

#endif // FISHINGMARKET_H
