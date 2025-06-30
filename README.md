# C Neural Evolution - Walking AI 🤖

This project is a 2D bipedal creature simulation that learns to walk using a neuroevolutionary algorithm. It features a custom physics engine and renders the simulation using SDL2.

## ✨ Features

-   **🧠 2D Physics Engine**: Custom-built using Verlet integration for points and constraints.
-   **🤖 Neural Network**: A simple feed-forward neural network that controls the creature's muscles.
-   **🧬 Genetic Algorithm**: Evolves the neural network weights over generations to improve walking ability.
-   **🖥️ SDL2 Rendering**: Visualizes the creatures and their movement.

## 🖼️ Screenshots


Screenshot 2025-06-30 132127.png

Screenshot 2025-06-30 132142.png

## 🛠️ Building and Running

### Prerequisites

You need to have `gcc` and `SDL2` installed on your system.

### Installation (Windows with MSYS2/MinGW)

1.  **Install MSYS2**: Follow the instructions at [msys2.org](https://www.msys2.org/).
2.  **Install `gcc` and `SDL2`**: Open the MSYS2 MinGW 64-bit terminal and run:
    ```bash
    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2
    ```
3.  **Update Makefile**: You will need to update the `CFLAGS` and `LDFLAGS` in the `Makefile` to point to your SDL2 installation. For a standard MSYS2 installation, they should be:
    ```makefile
    CFLAGS=-I/mingw64/include/SDL2 -O2 -Wall
    LDFLAGS=-L/mingw64/lib -lSDL2 -lm
    ```

### Compilation

Once your `Makefile` is configured, you can compile the project by running `make` in the root directory:

```bash
make
```

This will create an executable named `walking` (or `walking.exe` on Windows) in the root directory.

### Running

Execute the compiled program:

```bash
./walking
```

## ⚙️ How It Works

-   The simulation starts with a population of randomly generated bipeds.
-   Each biped has a neural network that takes sensory information (e.g., limb positions) as input and outputs forces to apply to its "muscles".
-   The simulation runs for a fixed duration (a "generation").
-   At the end of the generation, each biped is assigned a fitness score based on how far it traveled.
-   The genetic algorithm selects the fittest individuals (elitism), and creates a new generation through crossover (mixing the "genes"/network weights of parents) and mutation (randomly altering genes).
-   This process repeats, and over many generations, the bipeds should evolve to walk more effectively.
