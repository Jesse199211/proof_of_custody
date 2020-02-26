/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "openssl/err.h"
#include "openssl/ssl.h"

#include "LSSS/Share.h"
#include "config.h"

int input_YN()
{
  string str = "";
  do
  {
    cout << "Enter Y/N" << endl;
    cin >> str;
    if (str == "y" || str == "Y")
    {
      return 1;
    }
    if (str == "n" || str == "N")
    {
      return 0;
    }
  } while (0 == 0);
}

void init_certs()
{
  // Initialize the SSL library
  OPENSSL_init_ssl(
      OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  // SSL_CTX *ctx = InitServerCTX();

  ofstream output("Data/NetworkData.txt");

  cout << "Enter the name of the root CA (e.g. RootCA)" << endl;
  string CAName;
  cin >> CAName;

  string str = "Cert-Store/" + CAName + ".crt";
  ifstream cert_file(str);
  stringstream cert_buff;
  cert_buff << cert_file.rdbuf();
  cout << "Cert is\n"
       << cert_buff.str() << endl;
  BIO *bio_mem = BIO_new(BIO_s_mem());
  BIO_puts(bio_mem, cert_buff.str().c_str());
  X509 *x509 = PEM_read_bio_X509(bio_mem, NULL, NULL, NULL);
  EVP_PKEY *pkey = X509_get_pubkey(x509);
  BIO_free(bio_mem);
  X509_free(x509);
  cert_file.close();

  output << CAName << ".crt\n";

  int n = -1;
  while (n < 2 || n > 100)
  {
    cout << "Number of players (2--100) : " << endl;
    cin >> n;
  }
  output << n << endl;

  cout << "For each player now enter:\n";
  cout << "\t IP address (127.0.0.1) for localhost\n";
  cout << "\t The certificate name for this player\n";

  for (int i = 0; i < n; i++)
  {
    cout << "\n\nPlayer " << i << "\n";
    cout << "IP Address (in format "
            "127.0.0.1"
            ") "
         << endl;
    cin >> str;
    output << i << " " << str << " ";
    cout << "Name of certificate (e.g. "
            "Player1.crt"
            ")"
         << endl;
    cin >> str;
    output << str << " ";
    str = "Cert-Store/" + str;
    ifstream player_cert_file(str);
    stringstream player_cert_buff;
    player_cert_buff << player_cert_file.rdbuf();
    cout << "Cert is\n"
         << player_cert_buff.str() << endl;
    bio_mem = BIO_new(BIO_s_mem());
    BIO_puts(bio_mem, player_cert_buff.str().c_str());
    X509 *player_x509 = PEM_read_bio_X509(bio_mem, NULL, NULL, NULL);
    char buffer[256];
    X509_NAME_get_text_by_NID(X509_get_subject_name(player_x509), NID_commonName,
                              buffer, 256);
    cout << "Common name in certificate is " << buffer << endl;
    output << buffer << endl;
    int j = 0;
    while (buffer[j] != 0)
    {
      if (buffer[j] == ' ')
      {
        cout << "Common name may not contain spaces" << endl;
        abort();
      }
      j++;
    }
    int fl = X509_verify(player_x509, pkey);
    if (fl == 0)
    {
      cout << "Cerfificate does not verify" << endl;
      abort();
    }
    X509_free(player_x509);
    BIO_free(bio_mem);
    player_cert_file.close();
  }
  EVP_PKEY_free(pkey);

  /* XXXX
  cout << "Fake offline?" << endl;
  output << input_YN() << endl;
  cout << "Fake sacrifice?" << endl;
  output << input_YN() << endl;
*/
  // Choose non-fake in both cases
  output << 0 << endl;
  output << 0 << endl;

  output.close();
}

void enter_sets(imatrix &Sets, bool unqualified, unsigned int n)
{
  unsigned int v = 0;
  while (v < 1)
  {
    cout << "How many ";
    if (unqualified)
    {
      cout << "un";
    }
    cout << "qualified sets are you going to enter?\n";
    cin >> v;
  }
  Sets.resize(v);

  cout << "Each set is enterred as a vector of n 0/1 values\n";
  cout << "With 1 meaning that player is in the set\n";
  cout << "For example \n\t0 1 1 0\n";
  cout << "Would mean players 1 and 2 are the set, but players\n";
  cout << "zero and 3 are not." << endl;

  for (unsigned int i = 0; i < v; i++)
  {
    Sets[i].resize(n);
    bool ok = false;
    while (ok == false)
    {
      ok = true;
      cout << "Enter set number " << i << endl;
      for (unsigned int j = 0; j < n; j++)
      {
        cin >> Sets[i][j];
        if (Sets[i][j] != 0 && Sets[i][j] != 1)
        {
          ok = false;
        }
      }
      if (ok == false)
      {
        cout << "ERROR!\nTry Again!!!" << endl;
      }
    }
  }
}

void init_replicated(ShareData &SD, unsigned int n)
{
  cout << "How do you want to initialize the secret sharng scheme?\n";
  cout << "\t 1) Set of unqualified sets\n";
  cout << "\t 2) Set of qualified sets\n";
  cout << "\t 3) Replicated threshold structure";
  cout << endl;

  int v = -1;
  while (v < 1 || v > 3)
  {
    cout << "Enter a number (1-3).." << endl;
    cin >> v;
  }

  CAS AS;
  if (v == 1 || v == 2)
  {
    bool unqualified = true;
    if (v == 2)
    {
      unqualified = false;
    }
    imatrix Sets;
    enter_sets(Sets, unqualified, n);
    AS.assign(Sets, unqualified);
  }
  else
  {
    unsigned int t = n, bd = n / 2;
    if ((n & 1) == 1)
    {
      bd++;
    }
    while (t < 1 || t >= bd)
    {
      cout << "Enter threshold 0 < t < " << bd << endl;
      cin >> t;
    }
    AS.assign(n, t);
  }

  cout << "What type of Offline Phase do you want ?\n";
  cout << "\t 1) Maurer\n";
  cout << "\t 2) Reduced";
  cout << endl;

  v = -1;
  while (v < 1 || v > 2)
  {
    cout << "Enter a number (1-2).." << endl;
    cin >> v;
  }
  if (v == 1)
  {
    SD.Initialize_Replicated(AS, Maurer);
  }
  else
  {
    SD.Initialize_Replicated(AS, Reduced);
  }
}

void init_Q2_MSP(ShareData &SD, unsigned int n)
{
  vector<unsigned int> ns(n);
  cout << "Enter number of shares per player" << endl;
  unsigned int nr = 0;
  for (unsigned int i = 0; i < n; i++)
  {
    cout << "\t Player " << i << endl;
    cin >> ns[i];
    nr += ns[i];
  }

  cout << "Enter dimension of the MSP, i.e. number of columns of the generator "
          "matrix"
       << endl;
  unsigned int k;
  cin >> k;

  cout << "Now enter the matrix" << endl;
  gfp_matrix Gen(nr, vector<gfp>(k));
  for (unsigned int i = 0; i < nr; i++)
  {
    for (unsigned int j = 0; j < k; j++)
    {
      cin >> Gen[i][j];
    }
  }
  cout << "Generator matrix is: " << Gen << endl;
  cout << "Target vector is [1,1,...,1]" << endl;

  MSP M(Gen, ns);
  SD.Initialize_Q2(M);

  if (Gen.size() != SD.M.row_dim())
  {
    cout << "Note, the original MSP was not multiplicative" << endl;
    cout << "We therefore extended it to produce the following (equivalent) one"
         << endl;
    cout << SD.M << endl;
  }
}

void init_secret_sharing()
{
  ifstream input("Data/NetworkData.txt");
  string str;
  input >> str;
  unsigned int n;
  input >> n;
  input.close();

  cout << "Number of players = " << n << endl;

  cout << "\nChoose which LSSS you want..." << endl;
  cout << "\t1) Full Threshold\n";
  cout << "\t2) Shamir\n";
  cout << "\t3) Replicated\n";
  cout << "\t4) General Q2 MSP" << endl;

  unsigned int v = 0, t, bd;
  while (v < 1 || v > 4)
  {
    cout << "Enter2 a number (1-4).." << endl;
    cin >> v;
  }

  bigint p = 0;
  if (v == 1)
  {
    int answer = -1, lg2p = -1;
    while (answer != 0 && answer != 1)
    {
      cout << "Do you want me to find a prime for the LSSS, or do you want to try one yourself?\n";
      cout << "\t0) You find one\n\t1) I will enter one" << endl;
      cin >> answer;
    }
    if (answer == 0)
    {
      while (lg2p < 16 || lg2p > 1024)
      {
        cout << "What bit length of modulus do you want for secret sharing? (16-1024)" << endl;
        cin >> lg2p;
      }
    }
    else
    {
      cout << "Enter a prime for the LSSS.";
      cout << "Note if I cannot find FHE parameters suitable for this prime I will abort\n";
      cout << "This option is really for expert use only..." << endl;
      cin >> p;
    }
  }
  else
  {
    while (p < 2)
    {
      cout << "\nWhat modulus do you want to use for secret sharing?" << endl;
      cin >> p;
    }
    cout << "Using modulus p=" << p << endl;
    gfp::init_field(p);
  }

  ShareData SD;
  switch (v)
  {
  case 1:
    SD.Initialize_Full_Threshold(n);
    break;
  case 2:
    t = 0;
    bd = n / 2;
    if ((n & 1) == 1)
    {
      bd++;
    }
    while (t < 1 || t >= bd)
    {
      cout << "Enter threshold 0 < t < " << bd << endl;
      cin >> t;
    }
    SD.Initialize_Shamir(n, t);
    break;
  case 3:
    init_replicated(SD, n);
    break;
  case 4:
    init_Q2_MSP(SD, n);
    break;
  default:
    throw not_implemented();
  }

  ofstream out("Data/SharingData.txt");
  out << p << endl;
  out << SD;
  out.close();

  PRNG G;
  G.ReSeed(0);
  for (unsigned int i = 0; i < n; i++)
  {
    stringstream ss;
    ss << "Data/MKey-" << i << ".key";
    ofstream outk(ss.str().c_str());
    for (unsigned int j = 0; j < SD.nmacs; j++)
    {
      gfp aa;
      aa.randomize(G);
      outk << aa << " ";
    }
    outk << endl;
    outk.close();
  }

  cout << "Finished setting up secret sharing. \nThe underlying MSP is...\n"
       << SD.M << endl;
}

int main(int argc, const char *argv[])
{

  int ans = -1;
  if (argc == 1)
  {
    cout << "What do you want to set up?\n";
    cout << "\t1) Certs\n";
    cout << "\t2) Secret Sharing\n";
    cout << endl;

    while (ans < 1 || ans > 2)
    {
      cout << "Enter a number (1-2).." << endl;
      cin >> ans;
    }
  }
  else if (argc == 2)
  {
    ans = atoi(argv[1]);
    if (ans < 0 || ans > 2)
    {
      throw bad_value();
    }
  }

  if (ans == 1)
  {
    init_certs();
  }
  if (ans == 2)
  {
    init_secret_sharing();
  }
}
