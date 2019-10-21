/**
 * @file   Request.hpp
 * @author Diego Paiva
 * @date   13/11/2018
 *
 * Estrutura de dados que mantém as requisições (pickups e deliveries de cada usuário).
 */

#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

#include "Node.hpp"

class Request
{
public:
  Node *pickup;
  Node *delivery;

  Request(Node *pickup, Node *delivery)
  {
    this->pickup = pickup;
    this->delivery = delivery;
  };

  ~Request() {}

  float getTimeWindowMedian()
  {
    return getCriticalNode()->getTimeWindowMedian();
  }

  Node* getCriticalNode()
  {
    return this->isInbound() ? pickup : delivery;
  }

  bool isInbound()
  {
    return !isOutbound();
  }

  // TODO: Melhorar a lógica que determina se a requisição é outbound
  bool isOutbound()
  {
    return delivery->arrivalTime + delivery->departureTime != 1440;
  }
};

#endif // REQUEST_H_INCLUDED
