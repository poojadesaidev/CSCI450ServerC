#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <list>

using namespace std;

#define UDPPORT 23112
#define DOMAIN PF_INET // TODO change domain to AF_INET for Unix
#define MAXBUFLEN 4096
#define IPADDR "127.0.0.1"
#define FILENAME "block3.txt"
#define SERVERNAME "ServerC"

// create class for creating a sorted list of structs for TXLIST
struct Transaction
{
  string id;
  string sender;
  string rec;
  string amount;
  Transaction() {}
  Transaction(string iid, string isender, string irec, string iamount)
  {
    id = iid;
    sender = isender;
    rec = irec;
    amount = iamount;
  }

  // overload the < operator which will be used by the sort method of list
  bool operator<(const Transaction &transObj) const
  {
    return id < transObj.id;
  }
};

string encode(string originalString)
{

  int n = originalString.length();

  // declaring character array
  char original_char_array[n + 1];
  char encoded_char_array[n];

  // copying the contents of the
  // string to char array
  strcpy(original_char_array, originalString.c_str());

  for (int i = 0; i < n + 1; i++)
  {
    char c = original_char_array[i];
    if (isdigit(c))
    {
      c = c + 3;
      if (c > 57)
      {
        c = c - 10;
      }
    }
    else if (isalpha(c))
    {
      c = c + 3;

      if ((c > 90 && c < 97) || c > 122)
      {
        c = c - 26;
      }
    }
    encoded_char_array[i] = c;
  }

  string s = encoded_char_array;
  return s;
  // cout << s << s.length() << endl;
}

string decode(string encodedString)
{

  int n = encodedString.length();

  // declaring character array
  char original_char_array[n + 1];
  char encoded_char_array[n + 1];

  // copying the contents of the
  // string to char array
  strcpy(encoded_char_array, encodedString.c_str());

  for (int i = 0; i < n + 1; i++)
  {
    char c = encoded_char_array[i];
    //cout << encoded_char_array[i] << endl;
    if (isdigit(c))
    {
      c = c - 3;
      if (c < 48)
      {
        c = c + 10;
      }
    }
    else if (isalpha(c))
    {
      c = c - 3;

      if ((c < 97 && c > 90) || c < 65)
      {
        c = c + 26;
      }
    }
    original_char_array[i] = c;
  }

  string s = original_char_array;

  return s;
  // cout << s << s.length() << endl;
}

int createBindDatagramSocket()
{
  int dg_sock;
  // Create a DG Socket
  // Domain = AF_INET  Type = SOCK_DGRAM Protocol = 0
  // We let the transport layer decide the protocol based on 'Type'
  if ((dg_sock = socket(DOMAIN, SOCK_DGRAM, 0)) == -1)
  {
    cerr << "Datagram socket could not be created for " << SERVERNAME;
    return -1;
  }

  // Bind the socket to the IP/Port
  sockaddr_in dg_hint; // address (IPV4) for welcoming socket
  dg_hint.sin_family = DOMAIN;
  dg_hint.sin_port = htons(UDPPORT); // htons to do host to network translation for port#
  //inet_pton(DOMAIN, IPADDR, &dg_hint.sin_addr); // inet_pton to convert a number in our IP to array of integers
  inet_pton(DOMAIN, IPADDR, &dg_hint.sin_addr);

  //if ((bind(stream_welcoming_sock, DOMAIN, &stream_hint, sizeof(stream_hint))) == -1)
  if ((::bind(dg_sock, (const sockaddr *)&dg_hint, sizeof(dg_hint))) == -1)
  //if ((bind(stream_welcoming_sock, (sockaddr *)&stream_hint, sizeof(stream_hint))) == -1)
  {
    cerr << "Datagram socket IP/Port binding could not be done for " << SERVERNAME;
    return -1;
  }
  return dg_sock;
}

string getCreditDebitTotal(string user)
{
  int creditDebitTotal = 0;
  bool userFoundFlag = false;
  string singleLine;

  if (user.empty())
  {
    return "empty";
  }

  //cout << " User " << user << endl;

  ifstream block1(FILENAME);

  while (getline(block1, singleLine))
  {
    if (!singleLine.empty())
    {
      istringstream stringStream(singleLine);
      string words[4];

      for (int i = 0; i < 4; i++)
      {
        stringStream >> words[i];
      }

      if (words[3].empty())
      {
        return "empty";
      }
      int d;
      try
      {
        string decodedValue = decode(words[3]);
        d = stoi(decodedValue);
      }
      catch (...)
      {
        return "empty";
      }

      if (!words[1].empty() && words[1].compare(user) == 0)
      {
        userFoundFlag = true;
        creditDebitTotal = creditDebitTotal - d;
      }

      if (!words[2].empty() && words[2].compare(user) == 0)
      {
        userFoundFlag = true;
        creditDebitTotal = creditDebitTotal + d;
      }
    }
  }

  block1.close();

  if (!userFoundFlag)
  {
    return "empty";
  }
  return encode(to_string(creditDebitTotal));
}

string writeTransactionToFile(string transaction)
{
  if (transaction.empty())
  {
    return "empty";
  }
  fstream block1;

  block1.open(FILENAME, ios_base::app | ios_base::in);
  if (block1.is_open())
  {
    block1 << transaction << endl;
    return "success";
  }
  else
  {
    return "invalid";
  }
  block1.close();
}

string readFileToGetMaxSerialNum()
{
  int creditDebitTotal = 0;
  bool userFoundFlag = false;
  string singleLine;
  //TODO check if file exists
  ifstream block1(FILENAME);
  int maxTrns = -1;

  while (getline(block1, singleLine))
  {
    if (!singleLine.empty())
    {
      istringstream stringStream(singleLine);
      string transStr;
      stringStream >> transStr;
      int tn = stoi(transStr);
      if (tn > maxTrns)
      {
        maxTrns = tn;
      }
    }
  }

  block1.close();
  return to_string(maxTrns);
}

string readAllTransactions()
{
  string singleLine;
  ifstream block1(FILENAME);

  list<Transaction> allTrans;

  while (getline(block1, singleLine))
  {
    if (!singleLine.empty())
    {
      istringstream stringStream(singleLine);
      Transaction t = Transaction();

      stringStream >> t.id;
      stringStream >> t.sender;
      stringStream >> t.rec;
      stringStream >> t.amount;
      allTrans.push_back(t);
    }
  }
  block1.close();
  if (allTrans.empty())
  {
    return "empty";
  }

  // sort all transactions by transaction id
  allTrans.sort();

  string returnString = "";
  for (Transaction t : allTrans)
  {
    returnString = returnString + t.id + " " + t.sender + " " + t.rec + " " + t.amount + "\n";
  }

  return returnString;
}

string checkWallet(string request)
{
  istringstream stringStream(request);
  string userNameEncrptd;
  stringStream >> userNameEncrptd;
  stringStream >> userNameEncrptd;
  return getCreditDebitTotal(userNameEncrptd);
}

string logTransaction(string request)
{
  istringstream stringStream(request);
  string trnstnSingle;
  string trnstnComplete = "";
  stringStream >> trnstnSingle;
  while (stringStream >> trnstnSingle)
  {
    trnstnComplete = trnstnComplete + trnstnSingle + " ";
  }
  size_t end = trnstnComplete.find_last_not_of(" ");
  trnstnComplete = (end == std::string::npos) ? "" : trnstnComplete.substr(0, end + 1);

  return writeTransactionToFile(trnstnComplete);
}

string getMaxSerialNum()
{
  return readFileToGetMaxSerialNum();
}

string transactionList()
{
  return readAllTransactions();
}

int main()
{

  int dg_sock = createBindDatagramSocket();

  if (dg_sock == -1)
  {
    return -1;
  }

  cout << "The " << SERVERNAME << " is up and running using UDP on port " << UDPPORT << "." << endl;

  // Recieve message
  sockaddr_in client;
  socklen_t clientLen = sizeof(client);
  char buf[MAXBUFLEN];
  while (true)
  {
    // Clear the buffer and client
    memset(buf, 0, MAXBUFLEN);
    //memset(&client, 0, clientLen);

    // Recieve message
    int bytesRecv = recvfrom(dg_sock, buf, MAXBUFLEN - 1, 0, (sockaddr *)&client, &clientLen);
    if (bytesRecv == -1)
    {
      cerr << SERVERNAME << " could not recieve msg from Main Server" << endl;
      continue;
    }

    if (bytesRecv == 0)
    {
      cout << "Main Server did not send on " << SERVERNAME << endl;
      continue;
    }

    cout << "The " << SERVERNAME << " received a request from the Main Server." << endl;

    // Process Request

    string request = string(buf, 0, bytesRecv);

    istringstream stringStream(request);
    string serviceRequested;
    stringStream >> serviceRequested;

    string response;
    if (serviceRequested.compare("check") == 0)
    {
      // Check Wallet
      response = checkWallet(request);
    }
    else if (serviceRequested.compare("log") == 0)
    {
      // Log Transaction
      response = logTransaction(request);
    }
    else if (serviceRequested.compare("serialnum") == 0)
    {
      // Get max serial number
      response = getMaxSerialNum();
    }
    else if (serviceRequested.compare("list") == 0)
    {
      // List Transactions
      response = transactionList();
    }
    else
    {
      response = "invalid";
    }

    int n = response.length();
    char char_array[n + 1];
    strcpy(char_array, response.c_str());

    // Send message
    if (sendto(dg_sock, char_array, n + 1, 0, (sockaddr *)&client, clientLen) == -1)
    {
      cerr << "Error sending message from " << SERVERNAME << " to Main Server "
           << endl;
      continue;
    }
    else
    {
      cout << "The " << SERVERNAME << " finished sending the response to the Main Server." << endl;
    }
  }
  // Close the socket
  close(dg_sock);
  return 0;
}
