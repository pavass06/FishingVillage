#ifndef FIRM_H
#define FIRM_H

#include "Agent.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include "JobMarket.h"  // For JobPosting struct
#include <random>       // For random distributions

// Structure to record each sale transaction.
struct SaleRecord {
    double salePrice;
    double quantity;
};

class Firm : public Agent {
protected:
    int numberOfEmployees;    // Number of workers employed by the firm
    double stock;             // Current product inventory (in fish units)
    double priceLevel;        // Offered price per fish (e.g., ~5.1 pounds)
    double salesEfficiency;   // Sales efficiency factor (units each employee can sell)
    double jobPostMultiplier; // Multiplier for number of job posts
    double wageExpense;       // Computed as numberOfEmployees * clearing wage

    // Tracking actual sales.
    double totalRevenue;                  // Accumulated revenue from sales
    std::vector<SaleRecord> sales;        // List of sale transactions

public:
    // Constructor with parameters.
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

    // Revenue is based on actual sales.
    virtual double calculateRevenue() const {
         return totalRevenue;
    }

    // Record a sale: update revenue and log the sale.
    void addSale(double salePrice, double quantity) {
         double saleValue = salePrice * quantity;
         totalRevenue += saleValue;
         sales.push_back({salePrice, quantity});
    }

    // Reset sales records and revenue.
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

   
     virtual void updateStock() {
          // Calculate the total fish sold today from recorded sales.
          double sold = 0.0;
          for (const auto &record : sales) {
               sold += record.quantity;
          }
          // Compute remaining stock after sales.
          double remainingStock = stock - sold;
          // Production capacity: each employee can produce 2 fish per day.
          double productionCapacity = 2.0 * numberOfEmployees;
          // New stock is the sum of unsold fish and the produced fish.
          stock = std::max(remainingStock + productionCapacity, 0.0);
          // IMPORTANT: Reset sales so that next cycle only considers new sales.
          sales.clear();
     }


    // Modified calculateFishProduced(): production is min(stock, 2 × numberOfEmployees).
    virtual double calculateFishProduced() const {
         return std::min(stock, 2.0 * static_cast<double>(numberOfEmployees));
    }

    // In act(), we now update the stock using the updateStock() function.
    virtual void act() override {
         updateStock(); // Update stock based on profit reinvestment.
         // Update funds: add revenue minus wage expenses.
         funds += calculateRevenue() - wageExpense;
         // Optionally, reset sales records.
         // resetSales();
    }

    virtual void update() override {
         Agent::update();
    }

    virtual void print() const override {
         Agent::print();
#if verbose==1
         std::cout << "Employees: " << numberOfEmployees 
                   << " | Stock: " << stock 
                   << " | Price Level: " << priceLevel 
                   << " | Sales Efficiency: " << salesEfficiency 
                   << " | Job Post Multiplier: " << jobPostMultiplier 
                   << " | Wage Expense: " << wageExpense 
                   << " | Revenue: " << calculateRevenue() 
                   << " | Profit: " << calculateProfit() << std::endl;
         std::cout << "Fish Produced (Daily Output): " << calculateFishProduced() << std::endl;
         std::cout << "Sales Records:" << std::endl;
         for (const auto &record : sales) {
             std::cout << "  Price: " << record.salePrice 
                       << " | Quantity: " << record.quantity << std::endl;
         }
#endif
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
    
    // Pure virtual function; derived classes must implement it.
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const = 0;
};

#endif // FIRM_H
