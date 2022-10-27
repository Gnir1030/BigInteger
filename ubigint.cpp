// $Id: ubigint.cpp,v 1.4 2022-03-21 16:02:26-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "debug.h"
#include "ubigint.h"

ostream& operator<< (ostream& out, const vector<uint8_t>& vec) {
   //uint8_t a = 2;
   if(vec.size() == 0){
      return out << 0;
   }
   int count = 1;
   for (auto itor = vec.rbegin(); itor != vec.rend(); ++itor) {
      out << int (*itor);
      count++;
      if(count == 70){
         out << "\\" << endl;
         count = 1;
      }
   }
   return out;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   return out << "ubigint(" << that.uvalue << ")";
}


ubigint::ubigint (unsigned long that){
   if(that == 0){
      return;
   }
   uvalue.push_back(that);
   DEBUGF ('~', this << " -> " << uvalue)
}


ubigint::ubigint (const string& that){
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      uvalue.insert(uvalue.cbegin(), digit - '0');  
   }
   while (uvalue.size() > 0 and uvalue.back() == 0) uvalue.pop_back();
}


ubigint ubigint::operator+ (const ubigint& that) const {
   auto i = uvalue.begin();
   auto j = that.uvalue.begin();
   ubigint result;
   uint8_t carry = 0;
   while(i != uvalue.end() && j != that.uvalue.end()){
      if(*i + *j >= 10){
         result.uvalue.push_back(*i + *j - 10 + carry);
         carry = 1;
      }
      else{
         if(*i + *j + carry >= 10){
            result.uvalue.push_back(*i + *j - 10 + carry);
            carry = 1;
         }
         else{
            result.uvalue.push_back(*i + *j + carry);
            carry = 0;
         }
      }
      i++; j++;
   }

   if(i == uvalue.end()){
      while(j != that.uvalue.end()){
         if(*j + carry >= 10){
            result.uvalue.push_back(*j + carry - 10);
            carry = 1;
         }
         else{
            result.uvalue.push_back(*j + carry);
            carry = 0;
         }
         j++;
      }
   }
   else{
      while(i != uvalue.end()){
         if(*i + carry >= 10){
            result.uvalue.push_back(*i + carry - 10);
            carry = 1;
         }
         else{
            result.uvalue.push_back(*i + carry);
            carry = 0;
         }
         i++;
      }
   }
   if(carry == 1){
      result.uvalue.push_back(carry);
   }
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   auto i = uvalue.begin();
   auto j = that.uvalue.begin();
   ubigint result;
   uint8_t carry = 0;
   while(i != uvalue.end() && j != that.uvalue.end()){
      if(*i - *j < 0){
         result.uvalue.push_back(*i - *j + 10 - carry);
         carry = 1;
      }
      else{
         if(*i - *j - carry < 0){
            result.uvalue.push_back(*i - *j + 10 - carry);
            carry = 1;
         }
         else{
            result.uvalue.push_back(*i - *j - carry);
            carry = 0;
         }
      }
      i++; j++;
   }
   while(i != uvalue.end()){
      if(*i - carry < 0){
         result.uvalue.push_back(*i - carry + 10);
         carry = 1;
      }
      else{
         result.uvalue.push_back(*i - carry);
         carry = 0;
      }
      i++;
   }
   if(carry == 1){
      throw invalid_argument ("ubigint operator-: right is bigger");
   }
   while (result.uvalue.size() > 0 and result.uvalue.back() == 0) result.uvalue.pop_back();
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint sum;
   uint8_t count = 0;
   for(auto i : this->uvalue){
      uint8_t carry = 0;
      ubigint result;
      for(int k = count; k > 0; k--){
         result.uvalue.push_back(0);
      }
      for(auto j: that.uvalue){
         if(i * j >= 10){
            if(i * j%10 + carry >= 10){
               result.uvalue.push_back((i* j + carry)%10);
               carry = (i*j + carry)/10;
            }
            else{
               result.uvalue.push_back(i*j%10 + carry);
               carry = i*j/10;
            }
         }
         else{
            if(i * j + carry >= 10){
               result.uvalue.push_back((i * j + carry)%10);
               carry = (i * j + carry)/10;
            }
            else{
               result.uvalue.push_back(i * j + carry);
               carry = 0;
            }
         }
      }
      if(carry != 0){
         result.uvalue.push_back(carry);
      }
      count++;
      sum = sum + result;
   }
   return sum;
}

void ubigint::multiply_by_2() {
   vector<uint8_t> result;
   uint8_t carry = 0;
   for(auto i : this->uvalue){
      if(i * 2 >= 10){
         result.push_back(i*2%10 + carry);
         carry = 1;
      }
      else{
         result.push_back(i*2 + carry);
         carry = 0;
      }
   }
   if(carry == 1){
      result.push_back(carry);
   }
   this->uvalue = std::move(result);
}

void ubigint::divide_by_2() {
   vector<uint8_t> result;
   //uint8_t carry = 0;
   auto i = uvalue.begin();
   while(i + 1 != uvalue.end()){
      if(*(i+1) % 2 == 1){
         result.push_back(*i/2 + 5);
      }
      else{
         result.push_back(*i/2);
      }
      i++;
   }
   result.push_back(*i/2);
   while (result.size() > 0 and result.back() == 0) result.pop_back();
   this->uvalue = std::move(result);
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2(1);
   ubigint quotient{};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   return uvalue == that.uvalue;
}

bool ubigint::operator< (const ubigint& that) const {
   if(uvalue.size() < that.uvalue.size())
      return true;
   else if (uvalue.size() > that.uvalue.size())
      return false;
   auto i = uvalue.rbegin();
   auto j = that.uvalue.rbegin();
   while(i != uvalue.rend() && j != that.uvalue.rend()){
      if(*i < *j){
         return true;
      }
      else if (*i > *j){
         return false;
      }
      i++; j++;
   }
   return false;
}

void ubigint::print() const {
   DEBUGF ('p', this << " -> " << *this);
   cout << uvalue;
}

