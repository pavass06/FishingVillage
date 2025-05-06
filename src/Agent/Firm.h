// Firm.h
#ifndef FIRM_H
#define FIRM_H

#include "Agent.h"
#include "JobMarket.h"
#include "../Market/JobTypes.h" 
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <random>
#include <cmath>       // For std::ceil

// Structure to record each sale transaction.
struct SaleRecord {
    double salePrice;
    double quantity;
};

class Firm : public Agent {
protected:
    int numberOfEmployees;    // Current number of workers
    double stock;             // Product inventory
    double priceLevel;        // Price per unit
    double salesEfficiency;   // Units each employee can sell
    double jobPostMultiplier; // Multiplier for job posts
    double wageExpense;       // numberOfEmployees * clearing wage

    double totalRevenue;                // Accumulated revenue
    std::vector<SaleRecord> sales;      // Transactions
    int nsales;                         // Sales this cycle
    int cumulativeSales;                // Sales across all cycles

    double quitRate;                    // Dismissal probability

public:
    Firm(int id, double initFunds, int lifetime, int numberOfEmployees,
         double stock, double priceLevel,
         double salesEfficiency = 2.0, double jobPostMultiplier = 1.05,
         double quitRate = 0.1)
         : Agent(id, initFunds, lifetime),
           numberOfEmployees(numberOfEmployees),
           stock(stock),
           priceLevel(priceLevel),
           salesEfficiency(salesEfficiency),
           jobPostMultiplier(jobPostMultiplier),
           wageExpense(0.0),
           totalRevenue(0.0),
           quitRate(quitRate),
           nsales(0),
           cumulativeSales(0)
    {}

    virtual ~Firm() {}

    // Returns revenue for this cycle.
    virtual double calculateRevenue() const {
         return totalRevenue;
    }

    // Record a sale.
    void addSale(double salePrice, double quantity) {
         double saleValue = salePrice * quantity;
         totalRevenue += saleValue;
         sales.push_back({salePrice, quantity});
         nsales++;
         cumulativeSales++;
    }

    // Reset only this cycle’s sales.
    void resetSales() {
         totalRevenue = 0.0;
         sales.clear();
         nsales = 0;
         // cumulativeSales is NOT reset
    }

    void setWageExpense(double clearingWage) {
         wageExpense = numberOfEmployees * clearingWage;
    }

    virtual double calculateProfit() const {
         return calculateRevenue() - wageExpense;
    }

    // Update inventory.
    virtual void updateStock() {
         double sold = 0.0;
         for (const auto &record : sales) {
              sold += record.quantity;
         }
         double remainingStock = stock - sold;
         double productionCapacity = salesEfficiency * numberOfEmployees;
         stock = std::max(remainingStock + productionCapacity, 0.0);
         sales.clear();
    }

    // Random dismissals.
    void dismissEmployees(std::default_random_engine &generator) {
         std::binomial_distribution<int> dismissDist(numberOfEmployees, quitRate);
         int dismissals = dismissDist(generator);
         numberOfEmployees -= dismissals;
#if verbose==1
         std::cout << "Firm " << getID() << " dismissed " << dismissals 
                   << " employees. New count: " << numberOfEmployees << std::endl;
#endif
    }

    // Called each cycle.
    virtual void act() override {
         updateStock();
         funds += calculateRevenue() - wageExpense;
    }

    virtual JobPosting generateJobPosting(const std::string &sector,
                                          int eduReq,
                                          int expReq,
                                          int attract) const = 0;

    virtual void update() override {
         Agent::update();
    }

    virtual void print() const override {
         Agent::print();
#if verbose==1
         std::cout << "Employees: " << numberOfEmployees 
                   << " | Stock: " << stock 
                   << " | Price: " << priceLevel 
                   << " | SalesEff: " << salesEfficiency 
                   << " | WageExp: " << wageExpense 
                   << " | Revenue: " << calculateRevenue() 
                   << " | Profit: " << calculateProfit() << std::endl;
         std::cout << "Fish Prod: " << (salesEfficiency * numberOfEmployees) << std::endl;
         std::cout << "Sales Records:" << std::endl;
         for (const auto &r : sales) {
             std::cout << "  Price: " << r.salePrice 
                       << " | Qty: " << r.quantity << std::endl;
         }
#endif
    }

    // Getters / setters
    int getNumberOfEmployees() const { return numberOfEmployees; }
    void setNumberOfEmployees(int ne) { numberOfEmployees = ne; }

    double getStock() const { return stock; }
    void setStock(double s) { stock = s; }

    double getPriceLevel() const { return priceLevel; }
    void setPriceLevel(double p) { priceLevel = p; }

    int getSales() const { return nsales; }
    int getCumulativeSales() const { return cumulativeSales; }

    double getSalesEfficiency() const { return salesEfficiency; }
    void setSalesEfficiency(double se) { salesEfficiency = se; }

    double getJobPostMultiplier() const { return jobPostMultiplier; }
    void setJobPostMultiplier(double jpm) { jobPostMultiplier = jpm; }

    virtual double getRevenue() const { return calculateRevenue(); }
};

#endif // FIRM_H
