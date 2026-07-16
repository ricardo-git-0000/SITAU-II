#include "sw_fifo.h"
#include <atldef.h>

#include <stdio.h>

sw_fifo::sw_fifo(unsigned int size) {
  m_size = size;
  data = new dma_buffer_t [m_size];
  wr_idx = rd_idx = 0;
  m_used = 0;

  InitializeCriticalSection(&cs);
  InitializeConditionVariable(&rd_cv);
  InitializeConditionVariable(&wr_cv);
}

sw_fifo::~sw_fifo() {
  DeleteCriticalSection(&cs);
  if (data) {
    delete[] data;
    data = nullptr;
  }
}

//----

bool sw_fifo::_empty() {
  return InterlockedXor((LONG *) &m_used, 0) ? false : true;
}

bool sw_fifo::_full() {
  return InterlockedXor((LONG *) &m_used, m_size) ? false : true;
}

//-----

int sw_fifo::used(){
  int used;
  EnterCriticalSection(&cs);
  used = m_used;
  LeaveCriticalSection(&cs);
  return used;
}

//-----

void sw_fifo::put(const dma_buffer_t &b) {
  EnterCriticalSection(&cs);
  while (is_full()) {
    ::SleepConditionVariableCS(&wr_cv, &cs, INFINITE);
  }
  // this macro becomes a no-op in release mode
  ATLASSERT(!is_full());
  m_used++;
  data[wr_idx] = b;
  circ_inc(wr_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&rd_cv);
}

bool sw_fifo::put(const dma_buffer_t &b, DWORD timeout_ms) {
  
  EnterCriticalSection(&cs);
  if(is_full()) {
    SleepConditionVariableCS(&wr_cv, &cs, timeout_ms);
    if (is_full()) {
      LeaveCriticalSection(&cs);
      return false;
    }
  }
  
  ATLASSERT(!is_full()); // this macro becomes a no-op in release mode
  m_used++;
  data[wr_idx] = b;
  circ_inc(wr_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&rd_cv);
  return true;
}

bool sw_fifo::nb_put(const dma_buffer_t &b) {
  EnterCriticalSection(&cs);
  if (is_full()) {
    LeaveCriticalSection(&cs);
    return false;
  }

  ATLASSERT(!is_full());
  m_used++;
  data[wr_idx] = b;
  circ_inc(wr_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&rd_cv);
  return true;
}

int sw_fifo::nb_put(const dma_buffer_t *b, unsigned int n) {
  EnterCriticalSection(&cs);
  if (is_full()) {
    LeaveCriticalSection(&cs);
    return 0;
  }
  ATLASSERT(!is_full());

  unsigned int spare = m_size - m_used;
  unsigned int n_written = (n > spare) ? spare : n;
  const dma_buffer_t *src = b;
  unsigned int i; 

  m_used += n_written;
  for (i = 0; i < n_written; i++) {
    data[wr_idx] = *src++;
    circ_inc(wr_idx); // circular increment
  }
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&rd_cv);
  return n_written;
}



bool sw_fifo::nb_can_put(){
  bool ret;
  EnterCriticalSection(&cs);
  ret = !is_full();
  LeaveCriticalSection(&cs);
  return ret;
}

//----

dma_buffer_t sw_fifo::get() {
  dma_buffer_t b;
  EnterCriticalSection(&cs);
  while (is_empty()) {
    ::SleepConditionVariableCS(&rd_cv, &cs, INFINITE);
  }
  // this macro becomes a no-op in release mode
  ATLASSERT(!is_empty());
  m_used--;
  b = data[rd_idx];
  circ_inc(rd_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&wr_cv);
  return b;
}

bool sw_fifo::get(dma_buffer_t &b, DWORD timeout_ms) {
  
  EnterCriticalSection(&cs);
  if(is_empty()) {
    SleepConditionVariableCS(&rd_cv, &cs, timeout_ms);
    if (is_empty()) {
      LeaveCriticalSection(&cs);
      printf("[%s] returning because fifo is empty\n", __FUNCTION__);
      return false;
    }
  }
  
  ATLASSERT(!is_empty()); // this macro becomes a no-op in release mode
  m_used--;
  b = data[rd_idx];
  circ_inc(rd_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&wr_cv);
  return true;
}


bool sw_fifo::nb_get(dma_buffer_t &b) {
  EnterCriticalSection(&cs);
  if (is_empty()) {
    LeaveCriticalSection(&cs);
    return false;
  }

  ATLASSERT(!is_empty());
  m_used--;
  b = data[rd_idx];
  circ_inc(rd_idx); // circular increment
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&wr_cv);
  return true;
}

int sw_fifo::nb_get(dma_buffer_t *b, unsigned int n) {
  EnterCriticalSection(&cs);
  if (is_empty()) {
    LeaveCriticalSection(&cs);
    return 0;
  }
  ATLASSERT(!is_empty());

  int n_read = (n > m_used) ? m_used : n;
  dma_buffer_t *dst = b;
  int i;

  m_used -= n_read;
  for (i = 0; i < n_read; i++) {
    *dst++ = data[rd_idx];
    circ_inc(rd_idx); // circular increment
  }
  LeaveCriticalSection(&cs);

  ::WakeConditionVariable(&wr_cv);
  return n_read;
}




bool sw_fifo::nb_can_get(){
  bool ret;
  EnterCriticalSection(&cs);
  ret = !is_empty();
  LeaveCriticalSection(&cs);
  return ret;
}

//----

dma_buffer_t sw_fifo::peek() {  
  dma_buffer_t b;
  EnterCriticalSection(&cs);
  while (is_empty()) {
    ::SleepConditionVariableCS(&rd_cv, &cs, INFINITE);
  }
  ATLASSERT(!is_empty());
  b = data[rd_idx];
  LeaveCriticalSection(&cs);
  return b;
}

bool sw_fifo::nb_peek(dma_buffer_t &b) {  
  EnterCriticalSection(&cs);
  if (is_empty()) {
    LeaveCriticalSection(&cs);
    return false;
  }

  ATLASSERT(!is_empty());
  b = data[rd_idx];
  LeaveCriticalSection(&cs);

  return true;
}

bool sw_fifo::nb_can_peek() {  
  bool ret;
  EnterCriticalSection(&cs);
  ret = !is_empty();
  LeaveCriticalSection(&cs);
  return ret;
}

bool sw_fifo::nb_poke(const dma_buffer_t &b) {  
  EnterCriticalSection(&cs);
  if (is_full()) {
    LeaveCriticalSection(&cs);
    return false;
  }

  ATLASSERT(!is_full());
  data[wr_idx] = b;
  LeaveCriticalSection(&cs);

  return true;
}

//-----

void sw_fifo::flush() {  
  EnterCriticalSection(&cs);
  // destroy the buffer contents
  for(unsigned int i = 0; i < m_used; i++) {
    
  }
  wr_idx = rd_idx = 0;
  m_used = 0;
  LeaveCriticalSection(&cs);
}

void sw_fifo::unlock() {
  //printf("[%s] *\n", __FUNCTION__);
  ::WakeConditionVariable(&rd_cv);
  ::WakeConditionVariable(&wr_cv);
}

void sw_fifo::resize(unsigned int size) {
  unsigned int i;
  unsigned int new_wr_idx;
  unsigned int new_used;
  
  EnterCriticalSection(&cs);
  
  dma_buffer_t *new_data = new dma_buffer_t [size];
  new_used = (m_used > size)? size : m_used;
  new_wr_idx = 0;
  
  for(i = 0; i < new_used; i++) {
    new_data[new_wr_idx] = data[rd_idx];
    circ_inc(rd_idx);     // circular increment
    circ_inc(new_wr_idx); // circular increment
  }
  
  delete[] data;
  
  m_size = size;
  data = new_data;
  rd_idx = 0;
  wr_idx = new_wr_idx;
  m_used = new_used;
  
  LeaveCriticalSection(&cs);
}

//-----

int sw_fifo::wait_for_used(unsigned int n, int timeout_ms) {
  int ret;
  BOOL ok;
  EnterCriticalSection(&cs);
  while (m_used < n) {
    // If the time-out interval elapses, the function re-acquires the critical section and returns zero
    ok = ::SleepConditionVariableCS(&rd_cv, &cs, (DWORD)timeout_ms);
    if (!ok) {
      break;
    }
  }
  ret = m_used;
  LeaveCriticalSection(&cs);
  return ret;
}
