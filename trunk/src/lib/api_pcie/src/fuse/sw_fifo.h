#pragma once

#include <windows.h>

#include "dma_buffer.h"

class sw_fifo {
public:
  explicit sw_fifo(unsigned int size = 1);
  ~sw_fifo();

  void flush();
  void unlock();  // Force to awaken thread blocked due to a empty/full condition
  void resize(unsigned int size);

  void put(const dma_buffer_t &b);
  bool put(const dma_buffer_t &b, DWORD timeout_ms = INFINITE);
  bool nb_put(const dma_buffer_t &b);
  bool nb_can_put();

  dma_buffer_t get();
  bool get(dma_buffer_t &b, DWORD timeout_ms = INFINITE);
  bool nb_get(dma_buffer_t &b);
  bool nb_can_get();
  
  

  dma_buffer_t peek();
  bool nb_peek(dma_buffer_t &b);
  bool nb_can_peek();

  bool nb_poke(const dma_buffer_t &b);

  int nb_get(dma_buffer_t *b, unsigned int n = 1);
  int nb_put(const dma_buffer_t *b, unsigned int n = 1);

  int used();

  int size() const {
    return m_size;
  }

  int wait_for_used(unsigned int n, int timeout_ms);

private:

  bool _empty();
  bool _full();

  void request_update() {};


  inline void circ_inc(unsigned int &idx) {
    idx++; 
    idx = (idx < m_size) ? idx : 0;
  }

  inline bool is_empty() const {
    return (m_used == 0);
  }

  inline bool is_full() const {
    return (m_used == m_size);
  }

  CRITICAL_SECTION cs;
  CONDITION_VARIABLE rd_cv; // used to awaken a reading thread blocked due to a empty condition
  CONDITION_VARIABLE wr_cv; // used to awaken a writing thread blocked due to a full condition
  HANDLE used_event;

  unsigned int m_size;
  unsigned int wr_idx;
  unsigned int rd_idx;
  unsigned int m_used;
  unsigned int m_free;
  dma_buffer_t *data;
};
