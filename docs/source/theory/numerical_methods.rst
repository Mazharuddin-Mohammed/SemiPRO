Numerical Methods
=================

**Author: Dr. Mazharuddin Mohammed**

This section describes the numerical algorithms and computational methods used in SemiPRO.

Finite Difference Methods
-------------------------

Spatial Discretization
~~~~~~~~~~~~~~~~~~~~~~

The spatial domain is discretized using a uniform grid with spacing :math:`\Delta x`. The first and second derivatives are approximated using central differences:

.. math::
   \frac{\partial u}{\partial x} \bigg|_i \approx \frac{u_{i+1} - u_{i-1}}{2\Delta x}

.. math::
   \frac{\partial^2 u}{\partial x^2} \bigg|_i \approx \frac{u_{i+1} - 2u_i + u_{i-1}}{(\Delta x)^2}

For higher accuracy, fourth-order schemes can be used:

.. math::
   \frac{\partial u}{\partial x} \bigg|_i \approx \frac{-u_{i+2} + 8u_{i+1} - 8u_{i-1} + u_{i-2}}{12\Delta x}

Temporal Discretization
~~~~~~~~~~~~~~~~~~~~~~

For time-dependent problems, several schemes are available:

**Explicit Euler (Forward Euler):**

.. math::
   u_i^{n+1} = u_i^n + \Delta t \cdot f(u_i^n, t^n)

**Implicit Euler (Backward Euler):**

.. math::
   u_i^{n+1} = u_i^n + \Delta t \cdot f(u_i^{n+1}, t^{n+1})

**Crank-Nicolson (Trapezoidal):**

.. math::
   u_i^{n+1} = u_i^n + \frac{\Delta t}{2}[f(u_i^n, t^n) + f(u_i^{n+1}, t^{n+1})]

Stability Analysis
~~~~~~~~~~~~~~~~~

The stability of explicit schemes is governed by the CFL condition. For the diffusion equation:

.. math::
   \frac{\partial u}{\partial t} = D \frac{\partial^2 u}{\partial x^2}

The stability condition is:

.. math::
   \frac{D \Delta t}{(\Delta x)^2} \leq \frac{1}{2}

Finite Element Methods
---------------------

Weak Formulation
~~~~~~~~~~~~~~~~

Consider the boundary value problem:

.. math::
   -\nabla \cdot (D \nabla u) + au = f \quad \text{in } \Omega

.. math::
   u = g \quad \text{on } \Gamma_D

.. math::
   D \frac{\partial u}{\partial n} = h \quad \text{on } \Gamma_N

The weak formulation is: Find :math:`u \in H^1(\Omega)` such that:

.. math::
   \int_\Omega D \nabla u \cdot \nabla v \, d\Omega + \int_\Omega auv \, d\Omega = \int_\Omega fv \, d\Omega + \int_{\Gamma_N} hv \, d\Gamma

for all :math:`v \in H_0^1(\Omega)`.

Galerkin Method
~~~~~~~~~~~~~~

The solution is approximated as:

.. math::
   u_h(x) = \sum_{j=1}^N U_j \phi_j(x)

where :math:`\phi_j` are the basis functions and :math:`U_j` are the nodal values.

Substituting into the weak form gives the linear system:

.. math::
   \mathbf{K} \mathbf{U} = \mathbf{F}

where:

.. math::
   K_{ij} = \int_\Omega D \nabla \phi_i \cdot \nabla \phi_j \, d\Omega + \int_\Omega a\phi_i\phi_j \, d\Omega

.. math::
   F_i = \int_\Omega f\phi_i \, d\Omega + \int_{\Gamma_N} h\phi_i \, d\Gamma

Element Types
~~~~~~~~~~~~

**Linear Triangular Elements:**

Shape functions for a triangular element with vertices at :math:`(x_1, y_1)`, :math:`(x_2, y_2)`, :math:`(x_3, y_3)`:

.. math::
   N_1 = \frac{1}{2A}[(x_2 y_3 - x_3 y_2) + (y_2 - y_3)x + (x_3 - x_2)y]

.. math::
   N_2 = \frac{1}{2A}[(x_3 y_1 - x_1 y_3) + (y_3 - y_1)x + (x_1 - x_3)y]

.. math::
   N_3 = \frac{1}{2A}[(x_1 y_2 - x_2 y_1) + (y_1 - y_2)x + (x_2 - x_1)y]

where :math:`A` is the area of the triangle.

**Quadratic Elements:**

For higher accuracy, quadratic elements with 6 nodes can be used, providing second-order accuracy.

Monte Carlo Methods
-------------------

Ion Implantation Simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The trajectory of an ion is simulated using the binary collision approximation:

1. **Free flight**: The ion travels in a straight line until the next collision
2. **Collision**: The scattering angle is determined from the impact parameter
3. **Energy loss**: Electronic and nuclear stopping reduce the ion energy

The scattering angle in the center-of-mass system is:

.. math::
   \cos(\theta_{cm}) = 1 - \frac{2T_m}{T}

where :math:`T_m` is the maximum energy transfer and :math:`T` is the ion energy.

Random Number Generation
~~~~~~~~~~~~~~~~~~~~~~~~

High-quality random numbers are essential for Monte Carlo simulations. The linear congruential generator:

.. math::
   X_{n+1} = (aX_n + c) \bmod m

with carefully chosen parameters :math:`a`, :math:`c`, and :math:`m`.

For better quality, the Mersenne Twister algorithm is preferred.

Variance Reduction
~~~~~~~~~~~~~~~~~

Several techniques reduce the statistical uncertainty:

- **Importance sampling**: Sample more frequently in important regions
- **Stratified sampling**: Divide the domain into strata
- **Control variates**: Use correlated variables with known expectations

Level Set Methods
-----------------

Interface Tracking
~~~~~~~~~~~~~~~~~

The level set function :math:`\phi(x,t)` represents the interface as the zero level set:

.. math::
   \Gamma(t) = \{x : \phi(x,t) = 0\}

The evolution equation is:

.. math::
   \frac{\partial \phi}{\partial t} + F|\nabla \phi| = 0

where :math:`F` is the normal velocity of the interface.

Reinitialization
~~~~~~~~~~~~~~~~

To maintain the signed distance property, the level set function is periodically reinitialized:

.. math::
   \frac{\partial \phi}{\partial \tau} + \text{sign}(\phi_0)(|\nabla \phi| - 1) = 0

where :math:`\phi_0` is the initial level set function.

Etching and Deposition
~~~~~~~~~~~~~~~~~~~~~

For etching processes, the normal velocity is:

.. math::
   F = -R_{etch}(\theta, x)

where :math:`R_{etch}` depends on the local angle :math:`\theta` and position :math:`x`.

For deposition:

.. math::
   F = R_{dep}(\theta, x)

Adaptive Mesh Refinement
------------------------

Error Estimation
~~~~~~~~~~~~~~~

The local error is estimated using:

.. math::
   \eta_K = h_K^{p+1} \|f\|_{L^2(K)}

where :math:`h_K` is the element size, :math:`p` is the polynomial order, and :math:`f` is the residual.

Refinement Strategy
~~~~~~~~~~~~~~~~~~

Elements are marked for refinement if:

.. math::
   \eta_K > \alpha \max_j \eta_j

where :math:`\alpha` is the refinement parameter (typically 0.3-0.5).

Coarsening Strategy
~~~~~~~~~~~~~~~~~~

Elements are marked for coarsening if:

.. math::
   \eta_K < \beta \max_j \eta_j

where :math:`\beta` is the coarsening parameter (typically 0.1).

Linear System Solvers
---------------------

Direct Methods
~~~~~~~~~~~~~

For small to medium systems, direct methods are preferred:

**LU Decomposition:**

.. math::
   \mathbf{A} = \mathbf{L} \mathbf{U}

where :math:`\mathbf{L}` is lower triangular and :math:`\mathbf{U}` is upper triangular.

**Cholesky Decomposition** (for symmetric positive definite matrices):

.. math::
   \mathbf{A} = \mathbf{L} \mathbf{L}^T

Iterative Methods
~~~~~~~~~~~~~~~~

For large sparse systems, iterative methods are more efficient:

**Conjugate Gradient Method:**

For symmetric positive definite systems:

.. math::
   \mathbf{x}_{k+1} = \mathbf{x}_k + \alpha_k \mathbf{p}_k

.. math::
   \mathbf{r}_{k+1} = \mathbf{r}_k - \alpha_k \mathbf{A} \mathbf{p}_k

.. math::
   \mathbf{p}_{k+1} = \mathbf{r}_{k+1} + \beta_k \mathbf{p}_k

where:

.. math::
   \alpha_k = \frac{\mathbf{r}_k^T \mathbf{r}_k}{\mathbf{p}_k^T \mathbf{A} \mathbf{p}_k}

.. math::
   \beta_k = \frac{\mathbf{r}_{k+1}^T \mathbf{r}_{k+1}}{\mathbf{r}_k^T \mathbf{r}_k}

**GMRES Method:**

For general nonsymmetric systems, GMRES minimizes the residual over the Krylov subspace:

.. math::
   \mathcal{K}_m = \text{span}\{\mathbf{r}_0, \mathbf{A}\mathbf{r}_0, \mathbf{A}^2\mathbf{r}_0, \ldots, \mathbf{A}^{m-1}\mathbf{r}_0\}

Preconditioning
~~~~~~~~~~~~~~

To improve convergence, the system is preconditioned:

.. math::
   \mathbf{M}^{-1} \mathbf{A} \mathbf{x} = \mathbf{M}^{-1} \mathbf{b}

Common preconditioners include:
- **Jacobi**: :math:`\mathbf{M} = \text{diag}(\mathbf{A})`
- **Gauss-Seidel**: :math:`\mathbf{M} = \mathbf{L} + \mathbf{D}`
- **Incomplete LU**: :math:`\mathbf{M} = \tilde{\mathbf{L}} \tilde{\mathbf{U}}`

Parallel Computing
-----------------

Domain Decomposition
~~~~~~~~~~~~~~~~~~~

The computational domain is partitioned among processors:

.. math::
   \Omega = \bigcup_{i=1}^p \Omega_i

with minimal overlap and balanced load.

Message Passing
~~~~~~~~~~~~~~

Communication between processors uses MPI (Message Passing Interface):

- **Point-to-point**: Send/receive between specific processors
- **Collective**: Broadcast, reduce, gather operations
- **Non-blocking**: Overlap computation and communication

Load Balancing
~~~~~~~~~~~~~

Dynamic load balancing redistributes work to maintain efficiency:

.. math::
   \text{Efficiency} = \frac{T_{\text{serial}}}{p \cdot T_{\text{parallel}}}

where :math:`p` is the number of processors.

GPU Computing
~~~~~~~~~~~~

For highly parallel operations, GPU acceleration is used:

- **CUDA**: NVIDIA GPU programming model
- **OpenCL**: Cross-platform parallel computing
- **Memory hierarchy**: Global, shared, constant, texture memory

These numerical methods provide the computational foundation for accurate and efficient simulation in SemiPRO.
