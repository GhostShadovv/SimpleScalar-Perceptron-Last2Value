/*
 * sim-bpred.c - sample branch predictor simulator implementation
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 *
 * Copyright (C) 1994, 1995, 1996, 1997, 1998 by Todd M. Austin
 *
 * This source file is distributed "as is" in the hope that it will be
 * useful.  The tool index comes with no warranty, and no author or
 * distributor accepts any responsibility for the consequences of its
 * use.
 *
 * Everyone is granted permission to copy, modify and redistribute
 * this tool index under the following conditions:
 *
 *    This source code is distributed for non-commercial use only.
 *    Please contact the maintainer for restrictions applying to
 *    commercial use.
 *
 *    Permission is granted to anyone to make or distribute copies
 *    of this source code, either as received or modified, in any
 *    medium, provided that all copyright notices, permission and
 *    nonwarranty notices are preserved, and that the distributor
 *    grants the recipient permission for further redistribution as
 *    permitted by this document.
 *
 *    Permission is granted to distribute this file in compiled
 *    or executable form under the same conditions that apply for
 *    source code, provided that either:
 *
 *    A. it is accompanied by the corresponding machine-readable
 *       source code,
 *    B. it is accompanied by a written offer, with no time limit,
 *       to give anyone a machine-readable copy of the corresponding
 *       source code in return for reimbursement of the cost of
 *       distribution.  This written offer must permit verbatim
 *       duplication by anyone, or
 *    C. it is distributed by someone who received only the
 *       executable form, and is accompanied by a copy of the
 *       written offer of source code that they received concurrently.
 *
 * In other words, you are welcome to use, share and improve this
 * source file.  You are forbidden to forbid anyone else to use, share
 * and improve what you give them.
 *
 * INTERNET: dburger@cs.wisc.edu
 * US Mail:  1210 W. Dayton Street, Madison, WI 53706
 *
 * $Id: sim-perceptron.c,v 1.1.1.1 2000/05/26 15:19:03 taustin Exp $
 *
 * $Log: sim-perceptron.c,v $
 * Revision 1.1.1.1  2000/05/26 15:19:03  taustin
 * SimpleScalar Tool index
 *
 *
 * Revision 1.3  1999/12/31 18:46:55  taustin
 * quad_t naming conflicts removed
 *
 * Revision 1.2  1999/12/13 18:45:29  taustin
 * cross endian execution support added
 *
 * Revision 1.1  1998/08/27 15:54:35  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "options.h"
#include "stats.h"
#include "perceptron.h"
#include "sim.h"

#define MAX_PONDER 200

/*
 * This file implements a branch predictor analyzer.
 */


/* declaration of list of addresses for load value locality */
addrList l;

/* load value history */
static int history;
/* Total number of correctly predicted loads */
static counter_t valuePrediction = 0;

/* Total number of loads classified as predictable */
static counter_t classifiedPred = 0;

/* Total number of loads classified as unpredictable */
static counter_t classifiedUnpred = 0;

/* Total number of correctly classified predictable loads */
static counter_t predictable = 0;

/* Total number of correctly classified unpredictable loads */
static counter_t unpredictable = 0;

/* dimension of LVPT */
static int LVPTdim;

/* number of perceptrons */
static int PercepNr;

/* type of predictor */
static int contextual;

typedef struct LVPTelement *LVPTvalueList;
struct LVPTelement
{
  sword_t value;
  LVPTvalueList nextValue;
  int count;  // it's used only by the contextual predictor
};

typedef struct LVPTlocation *LVPTaddrList;
struct LVPTlocation
{
  md_addr_t addr;
  LVPTaddrList nextAddress;
  LVPTvalueList values;
  int automat;
  sword_t stride[2];  // it's used only by the stride predictor
};

/* dimension of VHT */
static int VHTdim ;

/* size of pattern */
static int pattern;

static int historyLength = 0;

/*value of threshold*/
static int threshold;

//miss in vht
static counter_t missVht = 0;

//wrong prediction
static counter_t nrWrong = 0;

//correct prediction
static counter_t nrCorrect = 0;

//correct prediction but value of putput less then the threshold
static counter_t nrCorrectThresh = 0;

//output from predictValue
static int Resolved_Output;

/* an element in a vht*/

// an entry in a vht
struct VHTlocation
{
  int tag;               /*address of load being tracked*/
  int LRU[2];
  int data_values[2];
  int vhp[MAX_PONDER];
  int vhpPatternSize;
  int used;
};
typedef struct VHTlocation  VHTlocation;
// vht table
struct PerceptronTableEntry
{
  int w[MAX_PONDER];
};
typedef struct PerceptronTableEntry  PerceptronTableEntry;

VHTlocation VHTtable[16384];
PerceptronTableEntry TablePerceptronLocation[16384];

void insertVHTValue(md_addr_t addr, int value)
{
  int i;
  //addr += 2;
  int index = addr % VHTdim;
  int indexPerceptron = addr % PercepNr;
  int tag = addr / VHTdim;
  historyLength = 1;
  VHTtable[index].tag = tag ;
  VHTtable[index].LRU[0] = 0;
  VHTtable[index].LRU[1] = 0;
  VHTtable[index].data_values[0] = value;
  VHTtable[index].data_values[1] = 0;
  VHTtable[index].used = 1;
  for(i = 0; i < historyLength; i++)
  {
        VHTtable[index].vhp[i] = 0;
        TablePerceptronLocation[indexPerceptron].w[i] = 0;
  }
  insertVHPValue(addr, value);
}

void insertVHPValue(md_addr_t addr, int value)
{
    int i;
    //addr +=2;
    int index = addr % VHTdim;
    int tag = addr / VHTdim;
    if(historyLength < history)
    {
      historyLength++;
    }

    for(i = 0; i < historyLength; i++)
    {
      VHTtable[index].vhp[i] = VHTtable[index].vhp[i+1];
    }
    VHTtable[index].vhp[historyLength] = value;

}

int valuePredict(md_addr_t addr)
{
        int i;
        //addr += 2;
        int index = addr % PercepNr;

        Resolved_Output = TablePerceptronLocation[index].w[0];
        for(i = 1; i <= historyLength; i++)
        {
            if(VHTtable[index].vhp[i] == 1)
            {
                Resolved_Output += TablePerceptronLocation[index].w[i];
            }
            else
            {
                if(VHTtable[index].vhp[i] == 0)
                {
                    Resolved_Output -= TablePerceptronLocation[index].w[i];
                }
            }
        }

        if(Resolved_Output >= 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
}

void learnPerceptron(int value, int valuePred, md_addr_t addrPC){
        int i;
        int index, indexPerceptron;
        int histCurent;
        int resolved_index;
        int change, T, Y;
        int lru;
        int vhp_temp[MAX_PONDER], hist_temp;
        //addrPC+=2;
        index = addrPC % VHTdim;
        indexPerceptron = addrPC % PercepNr;
        //predictie corecta
        if(valuePred == value)
        {
                nrCorrect++;
                if(abs(Resolved_Output) > threshold)
                {
                    nrCorrectThresh++;
                    if(value == VHTtable[index].data_values[0])
                    {
                        TablePerceptronLocation[indexPerceptron].w[0]--;
                        VHTtable[index].LRU[0]++;
                        resolved_index = 0;
                    }
                    else
                    {
                        if(value == VHTtable[index].data_values[1])
                        {
                            TablePerceptronLocation[indexPerceptron].w[0]++;
                            VHTtable[index].LRU[1]++;
                            resolved_index = 1;
                        }
                    }
                    for(i = 1; i <= historyLength; i++)
                    {
                        histCurent =  VHTtable[index].vhp[i];
                        if(histCurent == 0 && value == VHTtable[index].data_values[0])
                        {
                            TablePerceptronLocation[indexPerceptron].w[i]++;
                        }
                        if(histCurent == 1 && value == VHTtable[index].data_values[1])
                        {
                            TablePerceptronLocation[indexPerceptron].w[i]++;
                        }
                        if(histCurent == 0 && value == VHTtable[index].data_values[1])
                        {
                            TablePerceptronLocation[indexPerceptron].w[i]--;
                        }
                        if(histCurent == 1 && value == VHTtable[index].data_values[0])
                        {
                            TablePerceptronLocation[indexPerceptron].w[i]--;
                        }
                    }
                    insertVHPValue(addrPC, resolved_index);
                }
                else
                {
                    if(value == VHTtable[index].data_values[0])
                    {
                      VHTtable[index].LRU[0]++;
                      resolved_index = 0;
                    }
                    else if(value == VHTtable[index].data_values[1])
                      {
                          VHTtable[index].LRU[1]++;
                          resolved_index = 1;
                      }

                    insertVHPValue(addrPC, resolved_index);
                }
        }
        //predictie gresita
        else
        {
            nrWrong++;
            //valoarea generata este una din cele doua stocate in vht
            if(value == VHTtable[index].data_values[0] || valuePred == VHTtable[index].data_values[1])
            {
                if(value == VHTtable[index].data_values[0])
                {
                      VHTtable[index].LRU[0]++;
                      insertVHPValue(addrPC, 0);
                      T = -1;
                      Y = 1;
                }
                else
                {
                      VHTtable[index].LRU[0]++;
                      insertVHPValue(addrPC, 1);
                      T = 1;
                      Y = -1;
                }
                change = 2 * (T - Y);
                for(i = 1; i <= historyLength; i++)
                {
                    TablePerceptronLocation[indexPerceptron].w[i] += change;
                }
            }
            //mmiss in cache-ul de valori
            if(value != VHTtable[index].data_values[0] && valuePred != VHTtable[index].data_values[1])
            {
                if( VHTtable[index].LRU[0] < VHTtable[index].LRU[1])
                {
                    lru = 0;
                }
                else
                {
                    lru = 1;
                }
                if ((lru == 0 && TablePerceptronLocation[indexPerceptron].w[0] < 0) ||
                    (lru == 1 && TablePerceptronLocation[indexPerceptron].w[0] > 0))
                {
                    TablePerceptronLocation[indexPerceptron].w[0] = 0;
                }
                else
                {
                    if(lru == 0 && TablePerceptronLocation[indexPerceptron].w[0] >= 0)
                    {
                            TablePerceptronLocation[indexPerceptron].w[0]--;
                    }
                    if(lru == 1 && TablePerceptronLocation[indexPerceptron].w[0] < 0)
                    {
                            TablePerceptronLocation[indexPerceptron].w[0]++;
                    }
                }
                for(i = 0; i < historyLength; i++)
                {
                    TablePerceptronLocation[indexPerceptron].w[i] = 0;
                    vhp_temp[i] = VHTtable[index].vhp[i];
                }
                for(i = 0; i < historyLength; i++)
                {
                    VHTtable[index].vhp[i] = 0;
                }
                hist_temp = historyLength;
                historyLength = 0;
                for(i = 1; i < hist_temp; i++)
                {
                    if(vhp_temp[i] != lru)
                    {
                        insertVHPValue(addrPC, vhp_temp[i]);
                    }
                }
                insertVHPValue(addrPC, lru);
                VHTtable[index].data_values[lru] = value;
            }
        }


}

int foundVHTAddress(md_addr_t addr)
{
  int i;
  int found = 0;
  //addr+=2;
  int  index = addr % VHTdim;
  md_addr_t  tagg = addr / VHTdim;

  if(VHTtable[index].tag == tagg)
  {
      found = 1;
  }
  return found;
}

// VHT initialization
void VHTinit()
{
        int i, j, k;
        fprintf(stderr, "am intrat in vhtINit\n");
        for(i=0; i < VHTdim; i++)
        {
            VHTtable[i].vhpPatternSize = 0;
            //fprintf(stderr, "VHTtable[i].tag = -1 \n");
            VHTtable[i].tag = 0;
            //fprintf(stderr, "VHTtable[i].LRU[0] = 0\n");
            VHTtable[i].LRU[0] = 0;
            //fprintf(stderr, "VHTtable[i].LRU[1] = 0\n");
            VHTtable[i].LRU[1] = 0;
            //fprintf(stderr, "VHTtable[i].data_values[0] = -1;\n");
            VHTtable[i].data_values[0] = 0;
            //fprintf(stderr, "VHTtable[i].data_values[1] = -1;\n");
            VHTtable[i].data_values[1] = 0;
            VHTtable[i].used = 0;
            for(k = 0; k < MAX_PONDER; k++)
            {
                //fprintf(stderr, "VHTtable[i].vhp[k] = -1;\n");
                VHTtable[i].vhp[k] = 0;
            }

        }
}

// table of perceptrons initialization
void tableOfPerceptronsInit()
{
        int i, j, k;
        //fprintf(stderr, "am intrat in tableOfPerceptronsInit \n");
        for(i = 0; i < PercepNr; i++)
        {
            TablePerceptronLocation[i].w[0] = 0;
            //fprintf(stderr, "%d \n", i);
            for(k = 1; k < historyLength; k++)
            {
                TablePerceptronLocation[i].w[k] = 0;
            }
        }
}

LVPTaddrList pushLVPTAddress(LVPTaddrList p, md_addr_t addr)
{
  LVPTaddrList q;
  q=(LVPTaddrList)malloc(sizeof(struct LVPTlocation));
  q->addr=addr;
  q->nextAddress=p;
  q->values = NULL;
  q->automat = 0;
  return q;
}

void insertLVPTValue(LVPTaddrList ad, sword_t value, int history)
{
  int i;
  LVPTvalueList q = ad->values;
  LVPTvalueList p;
  LVPTvalueList newValue;
  if(history == 1)
  {
    if(ad->values != NULL)
    {
      if(!contextual)
      {
        ad->stride[1] = ad->stride[0];
        ad->stride[0] = value - q->value;
      }
      q->value = value;
      return;
    }
  }

  else if(q != NULL)
  {
  p = q->nextValue;
    for(i=1; i<history; i++)
  {
    if(p == NULL) break;
    if(i == history-1)
    {
      q->nextValue = NULL;
      free(p);
    break;
    }
    p=p->nextValue;
    q=q->nextValue;
  }

  }

  newValue=(LVPTvalueList)malloc(sizeof(struct LVPTelement));
  newValue->value=value;
  newValue->nextValue=ad->values;
  ad->values=newValue;

}

sword_t maxValue(LVPTvalueList p)
{
  LVPTvalueList q = p;
  int maximumCount = p->count;
  sword_t maximumValue = p->value;
  while(q != NULL)
  {
    if(q->count > maximumCount)
    {
      maximumCount = q->count;
      maximumValue = q->value;
    }
    q = q->nextValue;
  }
  return maximumValue;
}

sword_t predictValue(LVPTaddrList p, int history)
{
  int i,j;
  LVPTvalueList q = p->values;
  if(!contextual && p->stride[0] == p->stride[1])
    return q->value + p->stride[0];
  if(history == 1 && q != NULL)
    return q->value;
  else if(history>1 && q != NULL)  // contextual prediction
  {
    LVPTvalueList patternValueList = NULL;
    for(i=0; i<history-pattern; i++)
    {
      LVPTvalueList k;
      LVPTvalueList patternValues;
      int isPattern;
      if(q == NULL)
        break;
      k = q->nextValue;
      patternValues = p->values;
      isPattern = 1;
      for(j=0; j<pattern; j++)
      {
        if(k != NULL && patternValues != NULL)
        {
          if(k->value != patternValues->value)
          {
            isPattern = 0;
            break;
          }
        }
        else
        {
          isPattern = 0;
          break;
        }
        k = k->nextValue;
        patternValues = patternValues->nextValue;
      }
      if(isPattern)
      {
        LVPTvalueList newValue;
        if(patternValueList == NULL)  // insert the value
        {
          newValue=(LVPTvalueList)malloc(sizeof(struct LVPTelement));
          newValue->value = q->value; // the value which follows the pattern
          newValue->count = 1;
          newValue->nextValue = patternValueList;
          patternValueList = newValue;
        }
        else
        {
          int found = 0;
          LVPTvalueList temp = patternValueList;
          while(temp != NULL)
          {
            if(temp->value == q->value)
            {
              temp->count++;
              found = 1;
              break;
            }
            temp = temp->nextValue;
          }
          if(!found)  // insert the value
          {
            newValue=(LVPTvalueList)malloc(sizeof(struct LVPTelement));
            newValue->value = q->value; // the value which follows the pattern
            newValue->count = 1;
            newValue->nextValue = patternValueList;
            patternValueList = newValue;
          }
        }
      }
      q = q->nextValue;
    }
    if(patternValueList != NULL)
      return maxValue(patternValueList);  // contextual prediction
    else return p->values->value;  // the last value
  }
  return 0;
}

/* declaration of list */
LVPTaddrList lvpt;

int foundAssociativeLVPTAddress(md_addr_t addr, sword_t value, int history)
{

  LVPTaddrList p = lvpt;
  LVPTaddrList q;
  int i;
  int found = 0;
  if(lvpt == NULL)
    return found;
  if(p->addr == addr)
  {
    found = 1;
    if(p->values != NULL)
    {
      if(value == predictValue(p, history))
      {
        if(p->automat == 2 || p->automat == 3) /* predictable */
        {
          valuePrediction++;
          classifiedPred++;
          predictable++;
        }
        if(p->automat == 0 || p->automat == 1) /* unpredictable */
          classifiedUnpred++;
        if(p->automat < 3)
          p->automat++;
      }
      else
      {
        if(p->automat == 2 || p->automat == 3) /* predictable */
          classifiedPred++;
        if(p->automat == 0 || p->automat == 1) /* unpredictable */
        {
          classifiedUnpred++;
          unpredictable++;
        }
        if(p->automat > 0)
          p->automat--;
      }
    }
    else
    {
      if(p->automat > 0)
        p->automat--;
    }
    insertLVPTValue(p, value, history);
    return found;
  }

  else
  {
  q = p->nextAddress;
    for(i=1; i<LVPTdim; i++)
  {
    if(q == NULL) break;
    if(q->addr == addr)
    {
      found = 1;
      if(q->values != NULL)
      {
        if(value == predictValue(q, history))
        {
          if(q->automat == 2 || q->automat == 3) /* predictable */
          {
            valuePrediction++;
            classifiedPred++;
            predictable++;
          }
          if(q->automat == 0 || q->automat == 1) /* unpredictable */
            classifiedUnpred++;
          if(q->automat < 3)
            q->automat++;
        }
        else
        {
          if(q->automat == 2 || q->automat == 3) /* predictable */
            classifiedPred++;
          if(q->automat == 0 || q->automat == 1) /* unpredictable */
          {
            classifiedUnpred++;
            unpredictable++;
          }
          if(q->automat > 0)
            q->automat--;
        }
      }
      else
      {
        if(q->automat > 0)
          q->automat--;
      }
      insertLVPTValue(q, value, history);
      /* the address is moved to the first position in the list */
      p->nextAddress = q->nextAddress;
      q->nextAddress = lvpt;
      lvpt = q;
      break;
    }
    if(i == LVPTdim-1)
    {
      p->nextAddress = NULL;
      free(q);
    break;
    }
      p=p->nextAddress;
    q = q->nextAddress;
   }
   }

   return found;
}

/* LVPT initialization */
LVPTaddrList LVPTinit(LVPTaddrList l)
{
  int i;
  for(i=0; i<LVPTdim; i++)
    l = pushLVPTAddress(l, 0);
  return l;
}

void insertIntoDirrectMappedLVPT(md_addr_t addr, sword_t value, int history)
{

  LVPTaddrList p = lvpt;
  int index;
  int i;
  index = addr % LVPTdim;
  for(i=0; i<index; i++)
    p = p->nextAddress;
  if(p != NULL)
  {
    if(p->addr != addr)
    {
      p->addr = addr;
      p->values = NULL;
      p->automat = 0;
    }
    if(p->values != NULL)
    {
      if(value == predictValue(p, history))
      {
        if(p->automat == 2 || p->automat == 3) /* predictable */
        {
          valuePrediction++;
          classifiedPred++;
          predictable++;
        }
        if(p->automat == 0 || p->automat == 1) /* unpredictable */
          classifiedUnpred++;
        if(p->automat < 3)
          p->automat++;
      }
      else
      {
        if(p->automat == 2 || p->automat == 3) /* predictable */
          classifiedPred++;
        if(p->automat == 0 || p->automat == 1) /* unpredictable */
        {
          classifiedUnpred++;
          unpredictable++;
        }
        if(p->automat > 0)
          p->automat--;
      }
    }
    else
    {

      if(p->automat > 0)
        p->automat--;
    }
    insertLVPTValue(p, value, history);
  }

}

/* what to do? */
static int predict;  /* 1-prediction; 0-determining load value locality */

/* type of LVPT */
static int isAssoc;    /* 1-associative; 0-dirrect mapped */


/* which address? */
static int memAddress;  /* 1-memory address; 0-instruction address (PC); */

/* simulated registers */
static struct regs_t regs;

/* simulated memory */
static struct mem_t *mem = NULL;

/* maximum number of inst's to execute */
static unsigned int max_insts;

/* branch predictor type {nottaken|taken|perfect|bimod|2lev} */
static char *pred_type;

/* bimodal predictor config (<table_size>) */
static int bimod_nelt = 1;
static int bimod_config[1] =
  { /* bimod tbl size */2048 };

/* 2-level predictor config (<l1size> <l2size> <hist_size> <xor>) */
static int twolev_nelt = 4;
static int twolev_config[4] =
  { /* l1size */1, /* l2size */1024, /* hist */8, /* xor */FALSE};

/* combining predictor config (<meta_table_size> */
static int comb_nelt = 1;
static int comb_config[1] =
  { /* meta_table_size */1024 };

/* return address stack (RAS) size */
static int ras_size = 8;

/* BTB predictor config (<num_sets> <associativity>) */
static int btb_nelt = 2;
static int btb_config[2] =
  { /* nsets */512, /* assoc */4 };

/* branch predictor */
static struct bpred_t *pred;

/* track number of insn and refs */
static counter_t sim_num_refs = 0;

/* total number of branches executed */
static counter_t sim_num_branches = 0;

/* total number of loads executed */
static counter_t sim_num_loads = 0;


/* register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)
{
  opt_reg_header(odb,
"sim-bpred: This simulator implements a branch predictor analyzer.\n"
     );

  /* branch predictor options
  opt_reg_note(odb,
"  Branch predictor configuration examples for 2-level predictor:\n"
"    Configurations:   N, M, W, X\n"
"      N   # entries in first level (# of shift register(s))\n"
"      W   width of shift register(s)\n"
"      M   # entries in 2nd level (# of counters, or other FSM)\n"
"      X   (yes-1/no-0) xor history and address for 2nd level index\n"
"    Sample predictors:\n"
"      GAg     : 1, W, 2^W, 0\n"
"      GAp     : 1, W, M (M > 2^W), 0\n"
"      PAg     : N, W, 2^W, 0\n"
"      PAp     : N, W, M (M == 2^(N+W)), 0\n"
"      gshare  : 1, W, 2^W, 1\n"
"  Predictor `comb' combines a bimodal and a 2-level predictor.\n"
               );*/

  /* instruction limit */
  opt_reg_uint(odb, "-max:inst", "maximum number of inst's to execute",
               &max_insts, /* default */0,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-memaddr", "address type {0-instruction address| 1-memory address}",
               &memAddress, /* default */0,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-pred", "what to do {0-load value locality| 1-prediction}",
               &predict, /* default */1,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-assoc", "table type {0-dirrect mapped| 1-associative}",
               &isAssoc, /* default */0,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-history", "maximum number of values memorized for a certain address",
               &history, /* default */1,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-lvpt", "dimension of LVPT",
               &LVPTdim, /* default */4,
               /* print */TRUE, /* format */NULL);

 opt_reg_uint(odb, "-percep", "number of perceptrons",
              &PercepNr, /* default */16,
              /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-contextual", "type of predictor",
               &contextual, /* default */1,
               /* print */TRUE, /* format */NULL);

  opt_reg_string(odb, "-bpred",
                 "branch predictor type {nottaken|taken|bimod|2lev|comb}",
                 &pred_type, /* default */"bimod",
                 /* print */TRUE, /* format */NULL);

  opt_reg_int(odb, "-vht", "dimensiune VHT",
               &VHTdim, /* default */4096,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-pattern", "size of pattern",
               &pattern, /* default */32,
               /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-threshold", "threshold value",
               &threshold, /* default */10,
               /* print */TRUE, /* format */NULL);

}

/* check simulator-specific option addrList */
void
sim_check_options(struct opt_odb_t *odb, int argc, char **argv)
{
  if (!mystricmp(pred_type, "taken"))
    {
      /* static predictor, not taken */
      pred = bpred_create(BPredTaken, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  else if (!mystricmp(pred_type, "nottaken"))
    {
      /* static predictor, taken */
      pred = bpred_create(BPredNotTaken, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  else if (!mystricmp(pred_type, "bimod"))
    {
      if (bimod_nelt != 1)
  fatal("bad bimod predictor config (<table_size>)");
      if (btb_nelt != 2)
  fatal("bad btb config (<num_sets> <associativity>)");

      /* bimodal predictor, bpred_create() checks BTB_SIZE */
      pred = bpred_create(BPred2bit,
        /* bimod table size */bimod_config[0],
        /* 2lev l1 size */0,
        /* 2lev l2 size */0,
        /* meta table size */0,
        /* history reg size */0,
        /* history xor address */0,
        /* btb sets */btb_config[0],
        /* btb assoc */btb_config[1],
        /* ret-addr stack size */ras_size);
    }
  else if (!mystricmp(pred_type, "2lev"))
    {
      /* 2-level adaptive predictor, bpred_create() checks args */
      if (twolev_nelt != 4)
  fatal("bad 2-level pred config (<l1size> <l2size> <hist_size> <xor>)");
      if (btb_nelt != 2)
  fatal("bad btb config (<num_sets> <associativity>)");

      pred = bpred_create(BPred2Level,
        /* bimod table size */0,
        /* 2lev l1 size */twolev_config[0],
        /* 2lev l2 size */twolev_config[1],
        /* meta table size */0,
        /* history reg size */twolev_config[2],
        /* history xor address */twolev_config[3],
        /* btb sets */btb_config[0],
        /* btb assoc */btb_config[1],
        /* ret-addr stack size */ras_size);
    }
  else if (!mystricmp(pred_type, "comb"))
    {
      /* combining predictor, bpred_create() checks args */
      if (twolev_nelt != 4)
  fatal("bad 2-level pred config (<l1size> <l2size> <hist_size> <xor>)");
      if (bimod_nelt != 1)
  fatal("bad bimod predictor config (<table_size>)");
      if (comb_nelt != 1)
  fatal("bad combining predictor config (<meta_table_size>)");
      if (btb_nelt != 2)
  fatal("bad btb config (<num_sets> <associativity>)");

      pred = bpred_create(BPredComb,
        /* bimod table size */bimod_config[0],
        /* l1 size */twolev_config[0],
        /* l2 size */twolev_config[1],
        /* meta table size */comb_config[0],
        /* history reg size */twolev_config[2],
        /* history xor address */twolev_config[3],
        /* btb sets */btb_config[0],
        /* btb assoc */btb_config[1],
        /* ret-addr stack size */ras_size);
    }
  else
    fatal("cannot parse predictor type `%s'", pred_type);
}

/* register simulator-specific statistics */
void
sim_reg_stats(struct stat_sdb_t *sdb)
{
  stat_reg_counter(sdb, "sim_num_insn",
       "total number of instructions executed",
       &sim_num_insn, sim_num_insn, NULL);
    stat_reg_counter(sdb, "valuePrediction",
           "total number of correctly predicted values",
           &valuePrediction, valuePrediction, NULL);
  stat_reg_counter(sdb, "classifiedPred",
           "number of loads classified as predictable",
           &classifiedPred, classifiedPred, NULL);
  stat_reg_counter(sdb, "classifiedUnpred",
           "number of loads classified as unpredictable",
           &classifiedUnpred, classifiedUnpred, NULL);
  stat_reg_counter(sdb, "predictable",
           "correctly classified predictable loads",
           &predictable, predictable, NULL);
  stat_reg_counter(sdb, "unpredictable",
           "correctly classified unpredictable loads",
           &unpredictable, unpredictable, NULL);
  stat_reg_counter(sdb, "sim_num_refs",
       "total number of loads and stores executed",
       &sim_num_refs, 0, NULL);
  stat_reg_counter(sdb, "sim_num_loads",
       "total number of loads executed",
       &sim_num_loads, 0, NULL);
  stat_reg_int(sdb, "sim_elapsed_time",
         "total simulation time in seconds",
         &sim_elapsed_time, 0, NULL);
  stat_reg_formula(sdb, "sim_inst_rate",
       "simulation speed (in insts/sec)",
       "sim_num_insn / sim_elapsed_time", NULL);
  stat_reg_counter(sdb, "sim_num_branches",
                   "total number of branches executed",
                   &sim_num_branches, /* initial value */0, /* format */NULL);
  stat_reg_formula(sdb, "sim_IPB",
                   "instruction per branch",
                   "sim_num_insn / sim_num_branches", /* format */NULL);

   stat_reg_counter(sdb, "missVht",
                   "number of misses in vht",
                   &missVht, 0, NULL);
  stat_reg_counter(sdb, "nrWrong",
                   "total number of wrong predictions",
                   &nrWrong, 0, NULL);
  stat_reg_counter(sdb, "nrCorrect",
                   "total number of correct predictions",
                   &nrCorrect, 0, NULL);
  stat_reg_counter(sdb, "nrCorrectThresh",
                   "total number of correct predictions but the output was greater then the threshold",
                   &nrCorrectThresh, 0, NULL);

  /* register predictor stats */
//  if (pred)
//    bpred_reg_stats(pred, sdb);
}

/* initialize the simulator */
void
sim_init(void)
{
  sim_num_refs = 0;
  sim_num_loads = 0;
/*  loadValueLocality = 0; */

  /* allocate and initialize register file */
  regs_init(&regs);

  /* allocate and initialize memory space */
  mem = mem_create("mem");
  mem_init(mem);
}

/* local machine state accessor */
static char *          /* err str, NULL for no err */
bpred_mstate_obj(FILE *stream,      /* output stream */
     char *cmd,      /* optional command string */
     struct regs_t *regs,    /* register to access */
     struct mem_t *mem)    /* memory to access */
{
  /* just dump intermediate stats */
  sim_print_stats(stream);

  /* no error */
  return NULL;
}

/* load program into simulated state */
void
sim_load_prog(char *fname,    /* program to load */
        int argc, char **argv,  /* program arguments */
        char **envp)    /* program environment */
{
  /* load program text and data, index up environment, memory, and regs */
  ld_load_prog(fname, argc, argv, envp, &regs, mem, TRUE);

  /* initialize the DLite debugger */
  dlite_init(md_reg_obj, dlite_mem_obj, bpred_mstate_obj);
}

/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)    /* output stream */
{
  /* nothing currently */
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)    /* output stream */
{
  /* nada */
}

/* un-initialize simulator-specific state */
void
sim_uninit(void)
{
  /* nada */
}


/*
 * configure the execution engine
 */

/*
 * precise architected register accessors
 */

/* nextAddress program counter */
#define SET_NPC(EXPR)    (regs.regs_NPC = (EXPR))

/* target program counter */
#undef  SET_TPC
#define SET_TPC(EXPR)    (target_PC = (EXPR))

/* current program counter */
#define CPC      (regs.regs_PC)

/* general purpose registers */
#define GPR(N)      (regs.regs_R[N])
#define SET_GPR(N,EXPR)    (regs.regs_R[N] = (EXPR))

#if defined(TARGET_PISA)

/* general register dependence decoders */
#define DGPR(N)      (N)
#define DGPR_D(N)    ((N) &~1)

/* floating point register dependence decoders */
#define DFPR_L(N)    (((N)+32)&~1)
#define DFPR_F(N)    (((N)+32)&~1)
#define DFPR_D(N)    (((N)+32)&~1)

/* miscellaneous register dependence decoders */
#define DNA      (0)
#define DHI      (0+32+32)
#define DLO      (1+32+32)
#define DFCC      (2+32+32)
#define DTMP      (3+32+32)

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_L(N)    (regs.regs_F.l[(N)])
#define SET_FPR_L(N,EXPR)  (regs.regs_F.l[(N)] = (EXPR))
#define FPR_F(N)    (regs.regs_F.f[(N)])
#define SET_FPR_F(N,EXPR)  (regs.regs_F.f[(N)] = (EXPR))
#define FPR_D(N)    (regs.regs_F.d[(N) >> 1])
#define SET_FPR_D(N,EXPR)  (regs.regs_F.d[(N) >> 1] = (EXPR))

/* miscellaneous register accessors */
#define SET_HI(EXPR)    (regs.regs_C.hi = (EXPR))
#define HI      (regs.regs_C.hi)
#define SET_LO(EXPR)    (regs.regs_C.lo = (EXPR))
#define LO      (regs.regs_C.lo)
#define FCC      (regs.regs_C.fcc)
#define SET_FCC(EXPR)    (regs.regs_C.fcc = (EXPR))

#elif defined(TARGET_ALPHA)

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_Q(N)    (regs.regs_F.q[N])
#define SET_FPR_Q(N,EXPR)  (regs.regs_F.q[N] = (EXPR))
#define FPR(N)      (regs.regs_F.d[N])
#define SET_FPR(N,EXPR)    (regs.regs_F.d[N] = (EXPR))

/* miscellaneous register accessors */
#define FPCR      (regs.regs_C.fpcr)
#define SET_FPCR(EXPR)    (regs.regs_C.fpcr = (EXPR))
#define UNIQ      (regs.regs_C.uniq)
#define SET_UNIQ(EXPR)    (regs.regs_C.uniq = (EXPR))

#else
#error No ISA target defined...
#endif

/* precise architected memory state help functions */
#define READ_BYTE(SRC, FAULT)            \
  ((FAULT) = md_fault_none, MEM_READ_BYTE(mem, addr = (SRC)))
#define READ_HALF(SRC, FAULT)            \
  ((FAULT) = md_fault_none, MEM_READ_HALF(mem, addr = (SRC)))
#define READ_WORD(SRC, FAULT)            \
  ((FAULT) = md_fault_none, MEM_READ_WORD(mem, addr = (SRC)))
#ifdef HOST_HAS_QWORD
#define READ_QWORD(SRC, FAULT)            \
  ((FAULT) = md_fault_none, MEM_READ_QWORD(mem, addr = (SRC)))
#endif /* HOST_HAS_QWORD */

#define WRITE_BYTE(SRC, DST, FAULT)          \
  ((FAULT) = md_fault_none, MEM_WRITE_BYTE(mem, addr = (DST), (SRC)))
#define WRITE_HALF(SRC, DST, FAULT)          \
  ((FAULT) = md_fault_none, MEM_WRITE_HALF(mem, addr = (DST), (SRC)))
#define WRITE_WORD(SRC, DST, FAULT)          \
  ((FAULT) = md_fault_none, MEM_WRITE_WORD(mem, addr = (DST), (SRC)))
#ifdef HOST_HAS_QWORD
#define WRITE_QWORD(SRC, DST, FAULT)          \
  ((FAULT) = md_fault_none, MEM_WRITE_QWORD(mem, addr = (DST), (SRC)))
#endif /* HOST_HAS_QWORD */

/* system call handler macro */
#define SYSCALL(INST)  sys_syscall(&regs, mem_access, mem, INST, TRUE)

/* start simulation, program loaded, processor precise state initialized */
void
sim_main(void)
{
  int out1, out2;
  int in1, in2, in3;
  md_inst_t inst;
  register md_addr_t addr, target_PC;
  enum md_opcode op;
  register int is_write;
  int stack_idx;
  enum md_fault_type fault;
  int predValue;
  int i;

  fprintf(stderr, "sim: ** starting functional simulation w/ predictors **\n");

  /* index up initial default nextAddress PC */
  regs.regs_NPC = regs.regs_PC + sizeof(md_inst_t);

  /* check for DLite debugger entry condition */
  if (dlite_check_break(regs.regs_PC, /* no access */0, /* addr */0, 0, 0))
    dlite_main(regs.regs_PC - sizeof(md_inst_t), regs.regs_PC,
         sim_num_insn, &regs, mem);

  /* List initialization */
  l=news();
  lvpt=news();
  if(!isAssoc)
    lvpt=LVPTinit(lvpt);
  if(!contextual)  /* if it's an incremental prediction */
    history = 1;

  // List initialization
  fprintf(stderr, "am pornit initializarea pt VHT \n");
  VHTinit();

  fprintf(stderr, "am facut initializarea pt VHT, pornesc pt tabela de perceptroni \n");
  tableOfPerceptronsInit();


  while (TRUE)
    {

      /* maintain $r0 semantics */
      regs.regs_R[MD_REG_ZERO] = 0;
#ifdef TARGET_ALPHA
      regs.regs_F.d[MD_REG_ZERO] = 0.0;
#endif /* TARGET_ALPHA */

      /* get the nextAddress instruction to execute */
      MD_FETCH_INST(inst, mem, regs.regs_PC);

      /* keep an instruction count */
      sim_num_insn++;

      /* index default reference address and access mode */
      addr = 0; is_write = FALSE;

      /* index default fault - none */
      fault = md_fault_none;

      /* decode the instruction */
      MD_SET_OPCODE(op, inst);


    /* execute the instruction */
    switch (op)
    {

#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3)  \
  case OP:            \
          out1=O1;            \
          out2=O2;            \
          in1=I1;            \
          in2=I2;            \
          in3=I3;            \
    SYMCAT(OP,_IMPL);          \
          break;

#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)        \
        case OP:            \
          panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#define DECLARE_FAULT(FAULT)          \
    { fault = (FAULT); break; }
#include "machine.def"
  default:
    panic("attempted to execute a bogus opcode");
    }

      if (fault != md_fault_none)
  fatal("fault (%d) detected @ 0x%08p", fault, regs.regs_PC);


      if (MD_OP_FLAGS(op) & F_MEM)
  {
    sim_num_refs++;
    if (MD_OP_FLAGS(op) & F_STORE)
      is_write = TRUE;
  }

     /* keep a cond load instruction count */
        if(MD_OP_FLAGS(op) & F_LOAD)
        {
                sim_num_loads++;
            if(predict)	/* load value prediction */
            {
                if(isAssoc)
                {
                    if(memAddress)
                    {
                        if(!foundAssociativeLVPTAddress(addr, regs.regs_R[out1], history))
                        {
                            lvpt = pushLVPTAddress(lvpt, addr);
                            insertLVPTValue(lvpt, regs.regs_R[out1], history);
                        }
                    }
                    else
                    {
                        if(!foundAssociativeLVPTAddress(regs.regs_PC, regs.regs_R[out1], history))
                        {
                            lvpt = pushLVPTAddress(lvpt, regs.regs_PC);
                            insertLVPTValue(lvpt, regs.regs_R[out1], history);
                        }
                    }
                }
                else
                {
                    if(memAddress)
                    {
                        if(!foundVHTAddress(addr))
                        {
                            insertVHTValue(addr, regs.regs_R[out1]);
                            missVht++;
                            //nrWrong++;
                        }
                        else{
                            predValue = valuePredict(addr);
                            learnPerceptron(regs.regs_R[out1], VHTtable[addr % VHTdim].data_values[predValue], addr);
                        }
                        insertIntoDirrectMappedLVPT(addr, regs.regs_R[out1], history);
                    }
                    else
                    {
                        if(!foundVHTAddress(regs.regs_PC+2))
                        {
                            insertVHTValue(regs.regs_PC+2, regs.regs_R[out1]);
                            missVht++;
                            //nrWrong++;
                        }
                        else{
                            predValue = valuePredict(regs.regs_PC+2);
                            learnPerceptron(regs.regs_R[out1], VHTtable[(regs.regs_PC+2) % VHTdim].data_values[predValue], regs.regs_PC+2);
                        }
                        insertIntoDirrectMappedLVPT(regs.regs_PC, regs.regs_R[out1], history);
                    }
                }
            }
            else	/* load value locality verification */
            {
                // if(memAddress)
                // {
                //     if(l == NULL)
                //     l = pushAddress(l, addr, regs.regs_R[out1]);
                //     else if(!foundAddress(l, addr, regs.regs_R[out1], history))
                //     l = pushAddress(l, addr, regs.regs_R[out1]);
                // }
                // else
                // {
                //     if(l == NULL)
                //         l = pushAddress(l, regs.regs_PC, regs.regs_R[out1]);
                //     else if(!foundAddress(l, regs.regs_PC, regs.regs_R[out1], history))
                //         l = pushAddress(l, regs.regs_PC, regs.regs_R[out1]);
                // }
            }
        }


      if (MD_OP_FLAGS(op) & F_CTRL)
      {
          md_addr_t pred_PC;
          struct bpred_update_t update_rec;

          sim_num_branches++;

          if (pred)
          {
            /* get the nextAddress predicted fetch address */
            pred_PC = bpred_lookup(pred,
                 /* branch addr */regs.regs_PC,
                 /* target */target_PC,
                 /* inst opcode */op,
                 /* call? */MD_IS_CALL(op),
                 /* return? */MD_IS_RETURN(op),
                 /* stash an update ptr */&update_rec,
                 /* stash return stack ptr */&stack_idx);

            /* valid address returned from branch predictor? */
            if (!pred_PC)
            {
              /* no predicted taken target, attempt not taken target */
              pred_PC = regs.regs_PC + sizeof(md_inst_t);
            }

            bpred_update(pred,
             /* branch addr */regs.regs_PC,
             /* resolved branch target */regs.regs_NPC,
             /* taken? */regs.regs_NPC != (regs.regs_PC +
                   sizeof(md_inst_t)),
             /* pred taken? */pred_PC != (regs.regs_PC +
                  sizeof(md_inst_t)),
             /* correct pred? */pred_PC == regs.regs_NPC,
             /* opcode */op,
             /* predictor update pointer */&update_rec);
          }
      }

      /* check for DLite debugger entry condition */

      if (dlite_check_break(regs.regs_NPC,
          is_write ? ACCESS_WRITE : ACCESS_READ,
          addr, sim_num_insn, sim_num_insn))
            dlite_main(regs.regs_PC, regs.regs_NPC, sim_num_insn, &regs, mem);

      /* go to the nextAddress instruction */
      regs.regs_PC = regs.regs_NPC;
      regs.regs_NPC += sizeof(md_inst_t);

      /* finish early? */
      if (max_insts && sim_num_insn >= max_insts)
      {
          fprintf(stderr, "Acuratetea predictiei perceptron= %f \n", (nrCorrect * 100.0) / (sim_num_loads));
          fprintf(stderr, "Acuratetea predictiei load value= %f \n" , (classifiedPred * 100.0) / (sim_num_loads));
          int ctVhtLoad = 0, ct = 0;
          for(int i = 0; i < VHTdim; i++)
          {
            if(VHTtable[i].used == 1){
              ctVhtLoad++;
            }
            ct++;
          }
          float rez = (ctVhtLoad /(float)VHTdim)*100;
          fprintf(stderr, "Incarcare VHT Table %d/%d = %f \n" ,ctVhtLoad, VHTdim, rez);
          return;
      }
    }
}

