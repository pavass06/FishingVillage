#ifndef FIRM_H
#define FIRM_H

#include "Agent.h"
#include "JobMarket.h"
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
    int numberOfEmployees;    // Current number of workers employed by the firm
    double stock;             // Current product inventory (in fish units)
    double priceLevel;        // Offered price per fish (e.g., ~5.1 pounds)
    double salesEfficiency;   // Sales efficiency factor (units each employee can sell)
    double jobPostMultiplier; // Multiplier for the number of job posts
    double wageExpense;       // Computed as numberOfEmployees * clearing wage

    // Tracking actual sales.
    double totalRevenue;                  // Accumulated revenue from sales
    std::vector<SaleRecord> sales;        // List of sale transactions

    // pQuit chance: probability that an employee is dismissed (e.g., 0.1 for 10%)
    double quitRate;

public:
    // Constructor with parameters.
    // The targetEmployees is set initially equal to the starting number of employees.
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
           quitRate(quitRate)
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

    // Update stock based on sales and production.
    virtual void updateStock() {
          double sold = 0.0;
          for (const auto &record : sales) {
               sold += record.quantity;
          }
          double remainingStock = stock - sold;
          // Production: each employee produces (salesEfficiency) fish per day.
          double productionCapacity = salesEfficiency * numberOfEmployees;
          stock = std::max(remainingStock + productionCapacity, 0.0);
          // Reset sales for the next cycle.
          sales.clear();
    }

    // New method: shareholders (or an external process) may dismiss employees
    // with a chance of quitRate per employee.
    void dismissEmployees(std::default_random_engine &generator) {
         std::binomial_distribution<int> dismissDist(numberOfEmployees, quitRate);
         int dismissals = dismissDist(generator);
         numberOfEmployees -= dismissals;
#if verbose==1
         std::cout << "Firm " << " dismissed " << dismissals 
                   << " employees (pQuit process). New employee count: " 
                   << numberOfEmployees << std::endl;
#endif
    }

    // In act(), the firm now only updates its stock and funds.
    // The process of matching job applicants to vacancies and the subsequent hiring is handled in the JobMarket.
    virtual void act() override {
         updateStock();
         funds += calculateRevenue() - wageExpense;
         // Optionally, resetSales() may be called here if desired.
    }
    virtual JobPosting generateJobPosting(const std::string &sector, int eduReq, int expReq, int attract) const = 0;
    
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
         std::cout << "Fish Produced (Daily Output): " << (salesEfficiency * numberOfEmployees) << std::endl;
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
};

#endif // FIRM_H
