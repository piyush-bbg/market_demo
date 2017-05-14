#ifndef PGU_MESSAGES_HPP
#define PGU_MESSAGES_HPP
#include <iostream>

namespace pgu
{
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1) 

struct MarketStatus
{
  char type; // P
  char sym[4];
  short status;
  friend std::ostream& operator<<(std::ostream& os, const MarketStatus& t)
  {
    os << t.type << ", " << t.sym << ", " << t.status << std::endl;
    return os;
  }

};

struct Quote
{
  char type; // M
  int oid;
  char sym[4];
  double price;
  short side;
  short rank;

  friend std::ostream& operator<<(std::ostream& os,const Quote& t)
  {
    os << t.type << ", " << t.oid << ", " << t.sym << ", " << t.price << ", " << t.side << ", " << t.rank <<std::endl;
    return os;
  }

};

struct Trade 
{
  char type; // T 
  int tid;
  char sym[4];
  double price;
  
  friend std::ostream& operator<<(std::ostream& os, const Trade& t)
  {
    os << t.type << ", " << t.tid << ", " << t.sym << ", " << t.price << std::endl;
    return os;
  }

};

struct Logon
{
  char type; // L 
  int id;
  short subscription; // 1= phase  2 = price  4= trade
 
  friend std::ostream& operator<<(std::ostream& os, const Logon& t)
  {
    os << t.type << ", " << t.id << ", " << t.subscription <<std::endl;
    return os;
  }

};

#pragma pack(pop) 
} // namespace pgu ends

#endif
