# BookShelf Backend API

A high-performance backend recommendation engine and search API built with C++, PostgreSQL, and the Crow web framework. This project demonstrates the real-world application of complex data structures and algorithms in a microservices architecture.

## 🧠 Core Algorithms (DAA)

* **Collaborative Filtering (Bipartite Graph BFS):** Generates personalized book recommendations by performing a 2-hop traversal across a User-Book interaction graph. The algorithm uses frequency scoring to find mathematically relevant recommendations based on similar user tastes.
* **Search Autocomplete (Prefix Tree / Trie):** Provides lightning-fast, case-insensitive search suggestions. The database catalog is loaded into an in-memory Trie on startup, allowing for $O(L)$ time complexity lookups (where L is the prefix length).

## 🛠️ Tech Stack
* **Language:** C++17
* **Web Framework:** Crow (C++ Microframework)
* **Database:** PostgreSQL (with `libpqxx` driver)
* **Infrastructure:** Docker & Docker Compose
* **Scripting:** Python (for automated database seeding via Google Books API)

## 🚀 How to Run Locally

You do not need to install C++ or PostgreSQL on your machine to run this project. Everything is containerized.

**Prerequisites:** Ensure you have [Docker Desktop](https://www.docker.com/products/docker-desktop/) installed and running.

1. **Clone the repository and navigate into the folder:**
   ```bash
   git clone [https://github.com/bhaskarbajolaa-cmyk/BookShelf.git](https://github.com/bhaskarbajolaa-cmyk/BookShelf.git)
   cd BookShelf