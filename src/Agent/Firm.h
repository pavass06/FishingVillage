#ifndef FIRM_H
#define FIRM_H

#include "Agent.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include "JobMarket.h"  // For JobPosting struct

// Structure to record each sale transaction.
struct SaleRecord {
    double salePrice;
    double quantity;
};

class Firm : public Agent {
protected:
    int numberOfEmployees;    // Number of workers employed by the firm
    double stock;             // Current product inventory
    double priceLevel;        // Offered price per fish (e.g., ~6 pounds)
    double salesEfficiency;   // α: Sales efficiency factor (units each employee can sell)
    double jobPostMultiplier; // δ: Multiplier for number of job posts
    double wageExpense;       // Computed as numberOfEmployees * clearing wage

    // New members for tracking actual sales.
    double totalRevenue;                  // Accumulated revenue from actual sales
    std::vector<SaleRecord> sales;        // List of individual sale transactions

public:
    // Constructor with 8 parameters.
    Firm(int id, double initFunds, int lifetime, int numberOfEmployees,
         double stock, double priceLevel,
         double salesEfficiency = 2.0, double jobPostMultiplier = 1.05)
         : Agent(id, initFunds, lifetime),
           numberOfEmployees(numberOfEmployees),
           stock(stock),
           priceLevel(priceLevel),
           salesEfficiency(salesEfficiency),
           jobPostMultiplier(jobPostMultiplier),
           wageExpense(0.0),
           totalRevenue(0.0)
    {}

    virtual ~Firm() {}

    // Revenue now comes from the accumulated sales, not from assumed production.
    virtual double calculateRevenue() const {
         return totalRevenue;
    }

    // Add a sale transaction: update revenue and record the sale.
    void addSale(double salePrice, double quantity) {
         double saleValue = salePrice * quantity;
         totalRevenue += saleValue;
         sales.push_back({salePrice, quantity});
    }

    // Reset the sales records and revenue (if needed, e.g., at the start/end of a cycle).
    void resetSales() {
         totalRevenue = 0.0;
         sales.clear();
    }

    void setWageExpense(double clearingWage) {
         wageExpense = numberOfEmployees * clearingWage;
    }

    virtual double calculateProfit() const {
         return calculateRevenue() - wageExpense;
    }

    virtual double investmentExpenditure() const {
         double profit = calculateProfit();
         if (profit <= 0)
             return 0;
         double s = static_cast<double>(rand()) / RAND_MAX;
         return profit * (1.0 - s);
    }

    // New function to calculate the fish produced (potential output value)
    virtual double calculateFishProduced() const {
         // Compute the quantity of fish produced as the minimum of the available stock and twice the number of employees.
         double fishQuantity = std::min(stock, 2.0 * static_cast<double>(numberOfEmployees));
         return fishQuantity * priceLevel;
    }

    // In act(), revenue is now determined solely by actual sales recorded via addSale().
    virtual void act() override {
         double invest = investmentExpenditure();
         stock += invest;
         stock = std::max(stock, 0.0); // Ensure stock never goes negative.
         funds += calculateRevenue() - wageExpense;
         // Optionally, you might want to reset sales here if tracking per cycle.
         // resetSales();
    }

    virtual void update() override {
         Agent::update();
    }

    virtual void print() const override {
         Agent::print();
         std::cout << "Employees: " << numberOfEmployees 
                   << " | Stock: " << stock 
                   << " | Price Level: " << priceLevel 
                   << " | Sales Efficiency (α): " << salesEfficiency 
                   << " | Job Post Multiplier (δ): " << jobPostMultiplier 
                   << " | Wage Expense: " << wageExpense 
                   << " | Revenue: " << calculateRevenue() 
                   << " | Profit: " << calculateProfit() << std::endl;
         std::cout << "Fish Produced (Potential Output Value): " << calculateFishProduced() << std::endl;
         std::cout << "Sales Records:" << std::endl;
         for (const auto &record : sales) {
             std::cout << "  Price: " << record.salePrice 
                       << " | Quantity: " << record.quantity << std::endl;
         }
    }

    // Getters and setters.
    int getNumberOfEmployees() const { return numberOfEmployees; }
    void setNumberOfEmployees(int ne) { numberOfEmployees = ne; }

    double getStock() const { return stock; }
    void setStock(double s) { stock = s; }

    double getPriceLevel() const { return priceLevel; }
    void setPriceLevel(double p) { priceLevel = p; }

    double getSalesEfficiency() const { return salesEfficiency; }
    void setSalesEfficiency(double se) { salesEfficiency = se; }

    double getJobPostMultiplier() const { return jobPostMultiplier; }
    void setJobPostMultiplier(double jpm) { jobPostMultiplier = jpm; }

    virtual double getRevenue() const { return calculateRevenue(); }
    
    // Pure virtual method so that derived classes must implement it.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const = 0;
};

#endif // FIRM_H
