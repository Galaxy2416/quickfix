/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#endif

#include "quickfix/config.h"

#include "Application.h"
#include "quickfix/Session.h"
#include <iostream>
#include <chrono>

std::chrono::duration<double, std::milli> _tm;

void Application::onLogon( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logon - " << sessionID << std::endl;
}

void Application::onLogout( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logout - " << sessionID << std::endl;
}

void Application::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
EXCEPT( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
  crack( message, sessionID );
  std::cout << std::endl << "IN: " << message << std::endl;
  std::cout << std::endl << "time spent on sending: " << _tm.count() << " ms" << std::endl;
}

void Application::toApp( FIX::Message& message, const FIX::SessionID& sessionID )
EXCEPT( FIX::DoNotSend )
{
  try
  {
    FIX::PossDupFlag possDupFlag;
    message.getHeader().getField( possDupFlag );
    if ( possDupFlag ) throw FIX::DoNotSend();
  }
  catch ( FIX::FieldNotFound& ) {}

  std::cout << std::endl
  << "OUT: " << message << std::endl;
}

void Application::onMessage
( const FIX42::ExecutionReport&, const FIX::SessionID& ) 
{

  std::cout << std::endl << "The callback func of ExecutionReport: " << std::endl;
}
void Application::onMessage
( const FIX42::OrderCancelReject&, const FIX::SessionID& ) {}

void Application::run()
{
  while ( true )
  {
    try
    {
      char action = queryAction();

      if ( action == '1' )
        queryEnterOrder(1000);
      else if ( action == '2' )
      {
        queryEnterOrder(100);
	sleep(1);
	std::cout << std::endl << "Try to copy the tag 37 and cancel the order :) " << std::endl;
        queryCancelOrder();
      }
      else if ( action == '3' )
        queryReplaceOrder();
      else if ( action == '4' )
        queryManyOrders();
      else if ( action == '5' )
        break;
    }
    catch ( std::exception & e )
    {
      std::cout << "Message Not Sent: " << e.what();
    }
    sleep(1);
  }
}

void Application::queryEnterOrder(int qty)
{
  std::cout << "\nNewOrderSingle\n";
  FIX::Message order;
  order = queryNewOrderSingle42( qty );
  FIX::Session::sendToTarget( order );
}

void Application::queryCancelOrder()
{
  std::cout << "\nOrderCancelRequest\n";
  FIX::Message cancel;
  cancel = queryOrderCancelRequest42();
  FIX::Session::sendToTarget( cancel );
}

void Application::queryReplaceOrder()
{
  std::cout << "\nCancelReplaceRequest\n";
  FIX::Message replace;
  replace = queryCancelReplaceRequest42();
  FIX::Session::sendToTarget( replace );
}

void Application::queryManyOrders()
{
  size_t n = queryCount();
  std::cout << "\nGoing to Send " << n << "Orders \n";
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i != n; ++i)
  {
    std::cout << "Send the order number : " << n << std::endl;
    queryEnterOrder(100 * n + 1000);
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> tm = end - start;
  _tm = tm;
  std::cout << std::endl << "time spent: " << tm.count() << " ms" << std::endl;
}

FIX42::NewOrderSingle Application::queryNewOrderSingle42(int qty)
{
  FIX::OrdType ordType;

// Manual input mode
//  FIX42::NewOrderSingle newOrderSingle(
//    queryClOrdID(), FIX::HandlInst( '1' ), querySymbol(), querySide(),
//    FIX::TransactTime(), ordType = queryOrdType() );

  FIX42::NewOrderSingle newOrderSingle(
    FIX::ClOrdID("OrderID-2021-TESTING-NEW"), FIX::HandlInst( '1' ), FIX::Symbol("600000"), (FIX::Side_BUY),
    FIX::TransactTime(), ordType = FIX::OrdType( FIX::OrdType_LIMIT ));

  newOrderSingle.set(FIX::SecurityExchange("SH"));
  newOrderSingle.set(FIX::ClientID("yyn"));
  newOrderSingle.set(FIX::Account("1455018370591322112"));
  newOrderSingle.set(FIX::Currency("CNY"));
  newOrderSingle.setField(FIX::ContractNo("TestContractNo-28"));
  // newOrderSingle.set( queryOrderQty() );
  newOrderSingle.set(FIX::OrderQty(qty));
  newOrderSingle.set( FIX::TimeInForce( FIX::TimeInForce_DAY ));
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
  {
   // newOrderSingle.set( queryPrice() );
   newOrderSingle.set(FIX::Price(10.5));
  }
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX42::OrderCancelRequest Application::queryOrderCancelRequest42()
{
 // FIX42::OrderCancelRequest orderCancelRequest( queryOrigClOrdID(),
 //     queryClOrdID(), querySymbol(), querySide(), FIX::TransactTime() );
    FIX42::OrderCancelRequest orderCancelRequest(
      FIX::OrigClOrdID("OrderID-2021-TESTING-NEW"), FIX::ClOrdID("OrderID-2021-TESTING-Cancel"), FIX::Symbol("600000"), (FIX::Side_BUY),
      FIX::TransactTime());

  orderCancelRequest.set(FIX::SecurityExchange("SH"));
  // orderCancelRequest.set(FIX::OrderID("6a477d06-8959-4d14-be4f-f573167b067a"));
  // Input the order ID !
  orderCancelRequest.set(queryOrderID());
  orderCancelRequest.set(FIX::Account("auto-fundAccount"));

  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX42::OrderCancelReplaceRequest Application::queryCancelReplaceRequest42()
{
  FIX42::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::HandlInst( '1' ),
    querySymbol(), querySide(), FIX::TransactTime(), queryOrdType() );

    cancelReplaceRequest.set( queryPrice() );
    cancelReplaceRequest.set( queryOrderQty() );

  queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

void Application::queryHeader( FIX::Header& header )
{

  header.setField( FIX::SenderCompID("CLIENT") );
  header.setField( FIX::TargetCompID("SERVER") );
  /*
  header.setField( querySenderCompID() );
  header.setField( queryTargetCompID() );

  if ( queryConfirm( "Use a TargetSubID" ) )
    header.setField( queryTargetSubID() );
  */
}

char Application::queryAction()
{
  char value;
  std::cout << std::endl
  << "1) Enter Order" << std::endl
  << "2) Cancel Order" << std::endl
  << "3) Replace Order" << std::endl
  << "4) Many Many Orders Test" << std::endl
  << "5) Quit" << std::endl
  << "Action: ";
  std::cin >> value;
  switch ( value )
  {
    case '1': case '2': case '3': case '4': case '5': break;
    default: throw std::exception();
  }
  return value;
}


FIX::SenderCompID Application::querySenderCompID()
{
  std::string value;
  std::cout << std::endl << "SenderCompID: ";
  std::cin >> value;
  return FIX::SenderCompID( value );
}

FIX::TargetCompID Application::queryTargetCompID()
{
  std::string value;
  std::cout << std::endl << "TargetCompID: ";
  std::cin >> value;
  return FIX::TargetCompID( value );
}

FIX::TargetSubID Application::queryTargetSubID()
{
  std::string value;
  std::cout << std::endl << "TargetSubID: ";
  std::cin >> value;
  return FIX::TargetSubID( value );
}

FIX::ClOrdID Application::queryClOrdID()
{
  std::string value;
  std::cout << std::endl << "ClOrdID: ";
  std::cin >> value;
  return FIX::ClOrdID( value );
}

FIX::OrigClOrdID Application::queryOrigClOrdID()
{
  std::string value;
  std::cout << std::endl << "OrigClOrdID: ";
  std::cin >> value;
  return FIX::OrigClOrdID( value );
}


FIX::OrderID Application::queryOrderID()
{
  std::string value;
  std::cout << std::endl << "OrdID: ";
  std::cin >> value;
  return FIX::OrderID( value );
}

FIX::Symbol Application::querySymbol()
{
  std::string value;
  std::cout << std::endl << "Symbol: ";
  std::cin >> value;
  return FIX::Symbol( value );
}

FIX::Side Application::querySide()
{
  char value;
  std::cout << std::endl
  << "1) Buy" << std::endl
  << "2) Sell" << std::endl
  << "3) Sell Short" << std::endl
  << "4) Sell Short Exempt" << std::endl
  << "5) Cross" << std::endl
  << "6) Cross Short" << std::endl
  << "7) Cross Short Exempt" << std::endl
  << "Side: ";

  std::cin >> value;
  switch ( value )
  {
    case '1': return FIX::Side( FIX::Side_BUY );
    case '2': return FIX::Side( FIX::Side_SELL );
    case '3': return FIX::Side( FIX::Side_SELL_SHORT );
    case '4': return FIX::Side( FIX::Side_SELL_SHORT_EXEMPT );
    case '5': return FIX::Side( FIX::Side_CROSS );
    case '6': return FIX::Side( FIX::Side_CROSS_SHORT );
    case '7': return FIX::Side( 'A' );
    default: throw std::exception();
  }
}

FIX::OrderQty Application::queryOrderQty()
{
  long value;
  std::cout << std::endl << "OrderQty: ";
  std::cin >> value;
  return FIX::OrderQty( value );
}

size_t Application::queryCount()
{
  long value;
  std::cout << std::endl << "How many orders: ";
  std::cin >> value;
  return size_t(value);
}

FIX::OrdType Application::queryOrdType()
{
  char value;
  std::cout << std::endl
  << "1) Market" << std::endl
  << "2) Limit" << std::endl
  << "3) Stop" << std::endl
  << "4) Stop Limit" << std::endl
  << "OrdType: ";

  std::cin >> value;
  switch ( value )
  {
    case '1': return FIX::OrdType( FIX::OrdType_MARKET );
    case '2': return FIX::OrdType( FIX::OrdType_LIMIT );
    case '3': return FIX::OrdType( FIX::OrdType_STOP );
    case '4': return FIX::OrdType( FIX::OrdType_STOP_LIMIT );
    default: throw std::exception();
  }
}

FIX::Price Application::queryPrice()
{
  double value;
  std::cout << std::endl << "Price: ";
  std::cin >> value;
  return FIX::Price( value );
}

FIX::StopPx Application::queryStopPx()
{
  double value;
  std::cout << std::endl << "StopPx: ";
  std::cin >> value;
  return FIX::StopPx( value );
}

FIX::TimeInForce Application::queryTimeInForce()
{
  char value;
  std::cout << std::endl
  << "1) Day" << std::endl
  << "2) IOC" << std::endl
  << "3) OPG" << std::endl
  << "4) GTC" << std::endl
  << "5) GTX" << std::endl
  << "TimeInForce: ";

  std::cin >> value;
  switch ( value )
  {
    case '1': return FIX::TimeInForce( FIX::TimeInForce_DAY );
    case '2': return FIX::TimeInForce( FIX::TimeInForce_IMMEDIATE_OR_CANCEL );
    case '3': return FIX::TimeInForce( FIX::TimeInForce_AT_THE_OPENING );
    case '4': return FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_CANCEL );
    case '5': return FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_CROSSING );
    default: throw std::exception();
  }
}
