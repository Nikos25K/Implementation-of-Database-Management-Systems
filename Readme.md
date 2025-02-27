# Implementation of Database Management Systems 🗃️

Welcome to this repository containing three core projects that explore the internal workings of database systems, from heap file management to indexing with B+ trees and external sorting algorithms. Built in **C** and leveraging block-level file management, these assignments demonstrate foundational database concepts.

[![C](https://img.shields.io/badge/C-99%20Standard-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Makefile](https://img.shields.io/badge/Build-Makefile-brightgreen)](https://www.gnu.org/software/make/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 📚 Table of Contents
1. [Projects Overview](#projects-overview)
2. [Key Features](#key-features)
3. [Tech Stack](#tech-stack)

---

## Projects Overview

### 1. **Heap File Management**
Implementation of a heap file structure to manage records at the block level.
- **Key Features**:
  - Create, open, and close heap files.
  - Insert, retrieve, and update records.
  - Block-level memory management (pinning/unpinning, dirty flags).
  - [Detailed README](Heap_file_management/README.md)

### 2. **B+ Tree Indexing**
Efficient indexing using B+ trees for fast record retrieval.
- **Key Features**:
  - Create and manage B+ tree files.
  - Insert entries with primary key checks (no duplicates).
  - Search for records by `id` with logarithmic complexity.
  - [Detailed README](B+Tree/README.md)

### 3. **External Merge Sort**
External sorting algorithm for large datasets using chunking and multi-way merging.
- **Key Features**:
  - Split data into sorted chunks.
  - Merge chunks iteratively (b-way merge).
  - Optimized block I/O operations.
  - [Detailed README](External_merge_sort/README.md)

---

## Tech Stack
- **Language**: C (C99 standard)
- **Block Management**: Custom block-level library (`BF` for buffer management).
- **Tools**: GNU Make, Valgrind (for memory debugging).
- **Concepts**: File organization, indexing, sorting, memory optimization.

---