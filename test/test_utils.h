#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <test.h>

/**
 * alloc_utils.
 * */

void test_arena_expand();
void test_arena_reset();
void test_arena_del();
void test_roundup();
void test_ptr_aligned_size();
void test_total_size();
void test_arena_use();
void test_free_ptr_index();
void test_ptr_free();
void test_free_ptr_use();
void test_mmap_use();

/**
 * alloc.h
 * */

void test_alloc_new();
void test_alloc_del();
// void test_alloc_resize();

#endif
