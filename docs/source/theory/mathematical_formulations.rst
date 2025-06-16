Mathematical Formulations
=========================

**Author: Dr. Mazharuddin Mohammed**

This section provides detailed mathematical formulations for all physical models implemented in SemiPRO.

Oxidation Kinetics
------------------

Deal-Grove Linear-Parabolic Model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The growth of silicon dioxide follows the Deal-Grove model, which accounts for both linear and parabolic growth regimes:

.. math::
   x_o^2 + Ax_o = B(t + \tau)

where:
- :math:`x_o` is the oxide thickness
- :math:`t` is the oxidation time
- :math:`\tau` is the time shift for initial oxide
- :math:`A` is the linear rate constant
- :math:`B` is the parabolic rate constant

**Solution:**

.. math::
   x_o = \frac{A}{2}\left[\sqrt{1 + \frac{4B(t + \tau)}{A^2}} - 1\right]

**Rate Constants:**

The parabolic rate constant represents diffusion-limited growth:

.. math::
   B = \frac{2DC^*}{N_0}

The linear rate constant represents surface reaction-limited growth:

.. math::
   A = \frac{2DC^*}{k_s N_0}

where:
- :math:`D` is the diffusivity of oxidizing species in SiO₂
- :math:`C^*` is the equilibrium concentration of oxidizing species
- :math:`N_0` is the number of oxidant molecules per unit volume of oxide
- :math:`k_s` is the surface reaction rate constant

**Temperature Dependence:**

Both rate constants follow Arrhenius behavior:

.. math::
   B(T) = B_0 \exp\left(-\frac{E_B}{k_B T}\right)

.. math::
   A(T) = A_0 \exp\left(-\frac{E_A}{k_B T}\right)

Ion Implantation
----------------

Range Statistics
~~~~~~~~~~~~~~~

**Projected Range:**

.. math::
   R_p = \int_0^{E_0} \frac{dE}{N S_n(E) + N S_e(E)}

where:
- :math:`E_0` is the initial ion energy
- :math:`N` is the atomic density of the target
- :math:`S_n(E)` is the nuclear stopping power
- :math:`S_e(E)` is the electronic stopping power

**Range Straggling:**

.. math::
   \Delta R_p^2 = \int_0^{E_0} \frac{S_n(E)}{[N S_n(E) + N S_e(E)]^3} dE

**Lateral Straggling:**

.. math::
   \Delta R_\perp^2 = \int_0^{E_0} \frac{S_n(E) \sin^2(\theta)}{[N S_n(E) + N S_e(E)]^3} dE

**Gaussian Approximation:**

For most practical cases, the implanted profile can be approximated as:

.. math::
   C(x) = \frac{\Phi}{\sqrt{2\pi}\Delta R_p} \exp\left(-\frac{(x-R_p)^2}{2\Delta R_p^2}\right)

where :math:`\Phi` is the implanted dose.

**Pearson IV Distribution:**

For more accurate modeling, especially for light ions:

.. math::
   C(x) = C_0 \left[1 + \left(\frac{x-R_p}{\alpha}\right)^2\right]^{-m} \exp\left(-\nu \arctan\left(\frac{x-R_p}{\alpha}\right)\right)

Stopping Power Models
~~~~~~~~~~~~~~~~~~~~

**Nuclear Stopping Power (LSS Theory):**

.. math::
   S_n = \frac{4\pi e^2 Z_1 Z_2}{m_1 + m_2} \frac{m_2}{m_1} \frac{a}{E} f(\epsilon)

where:
- :math:`Z_1, Z_2` are atomic numbers of ion and target
- :math:`m_1, m_2` are atomic masses
- :math:`a` is the screening length
- :math:`f(\epsilon)` is the reduced energy function

**Screening Length:**

.. math::
   a = \frac{0.885 a_0}{(Z_1^{2/3} + Z_2^{2/3})^{1/2}}

**Electronic Stopping Power:**

.. math::
   S_e = k E^n

where :math:`k` and :math:`n` are material-dependent constants.

Diffusion Processes
-------------------

Fick's Laws
~~~~~~~~~~

**First Law:**

.. math::
   \vec{J} = -D \nabla C

**Second Law:**

.. math::
   \frac{\partial C}{\partial t} = \nabla \cdot (D \nabla C) + G - R

where:
- :math:`C` is the concentration
- :math:`D` is the diffusivity
- :math:`G` is the generation rate
- :math:`R` is the recombination rate

**Concentration-Dependent Diffusion:**

.. math::
   D_{eff} = D_i + D_e \frac{n}{n_i}

where:
- :math:`D_i` is the intrinsic diffusivity
- :math:`D_e` is the extrinsic diffusivity
- :math:`n` is the electron concentration
- :math:`n_i` is the intrinsic carrier concentration

**Segregation at Interfaces:**

.. math::
   \frac{C_1}{C_2} = k_{seg}

where :math:`k_{seg}` is the segregation coefficient.

Photolithography
----------------

Light Propagation
~~~~~~~~~~~~~~~~

**Beer-Lambert Law:**

.. math::
   I(z) = I_0 \exp(-\alpha z)

where:
- :math:`I(z)` is the intensity at depth :math:`z`
- :math:`I_0` is the incident intensity
- :math:`\alpha` is the absorption coefficient

**Standing Wave Effects:**

.. math::
   I(z) = I_0[1 + m \cos(4\pi nz/\lambda + \phi)]

where:
- :math:`m` is the modulation depth
- :math:`n` is the refractive index
- :math:`\lambda` is the wavelength
- :math:`\phi` is the phase shift

Resist Chemistry
~~~~~~~~~~~~~~~

**Photochemical Reaction:**

.. math::
   \frac{dM}{dt} = -\sigma I(z) M

where:
- :math:`M` is the concentration of photosensitive compound
- :math:`\sigma` is the absorption cross-section

**Development Rate:**

.. math::
   R_{dev} = R_{max} \left(\frac{M_0 - M}{M_0}\right)^n

where:
- :math:`R_{max}` is the maximum development rate
- :math:`n` is the contrast parameter

Deposition Processes
--------------------

Chemical Vapor Deposition
~~~~~~~~~~~~~~~~~~~~~~~~~

**Overall Rate:**

.. math::
   R_{dep} = \frac{1}{\frac{1}{k_s} + \frac{1}{k_m}}

where:
- :math:`k_s` is the surface reaction rate
- :math:`k_m` is the mass transport rate

**Surface Reaction Rate:**

.. math::
   k_s = k_0 \exp\left(-\frac{E_a}{k_B T}\right) P^n

where:
- :math:`k_0` is the pre-exponential factor
- :math:`E_a` is the activation energy
- :math:`P` is the partial pressure
- :math:`n` is the reaction order

**Mass Transport Rate:**

.. math::
   k_m = \frac{D_{gas}}{L} \frac{P}{k_B T}

where:
- :math:`D_{gas}` is the gas phase diffusivity
- :math:`L` is the boundary layer thickness

**Step Coverage:**

.. math::
   S = \frac{t_{sidewall}}{t_{top}}

For conformal deposition: :math:`S = 1`

Etching Processes
-----------------

Plasma Etching
~~~~~~~~~~~~~

**Etch Rate:**

.. math::
   R_{etch} = Y \Gamma_{ion} + k_{chem} n_{radical}

where:
- :math:`Y` is the sputter yield
- :math:`\Gamma_{ion}` is the ion flux
- :math:`k_{chem}` is the chemical etch rate constant
- :math:`n_{radical}` is the radical density

**Sputter Yield:**

.. math::
   Y = \frac{3\alpha}{4\pi^2} \frac{4M_1 M_2}{(M_1 + M_2)^2} \frac{E - E_{th}}{U_0}

where:
- :math:`\alpha` is a material constant
- :math:`M_1, M_2` are ion and target masses
- :math:`E` is the ion energy
- :math:`E_{th}` is the threshold energy
- :math:`U_0` is the surface binding energy

**Selectivity:**

.. math::
   S = \frac{R_{etch,1}}{R_{etch,2}}

**Anisotropy:**

.. math::
   A = 1 - \frac{R_{lateral}}{R_{vertical}}

Thermal Analysis
----------------

Heat Conduction
~~~~~~~~~~~~~~

**Heat Equation:**

.. math::
   \rho c_p \frac{\partial T}{\partial t} = \nabla \cdot (k \nabla T) + Q

where:
- :math:`\rho` is the density
- :math:`c_p` is the specific heat capacity
- :math:`k` is the thermal conductivity
- :math:`Q` is the heat generation rate

**Joule Heating:**

.. math::
   Q_{Joule} = \vec{J} \cdot \vec{E} = \sigma E^2

where :math:`\sigma` is the electrical conductivity.

**Boundary Conditions:**

At surfaces:

.. math::
   -k \frac{\partial T}{\partial n} = h(T - T_{ambient}) + \epsilon \sigma_{SB}(T^4 - T_{ambient}^4)

where:
- :math:`h` is the convection coefficient
- :math:`\epsilon` is the emissivity
- :math:`\sigma_{SB}` is the Stefan-Boltzmann constant

Electromigration
---------------

**Flux Equation:**

.. math::
   \vec{J}_{EM} = \frac{D^* Z^* e \rho \vec{j}}{k_B T}

where:
- :math:`D^*` is the effective diffusivity
- :math:`Z^*` is the effective charge
- :math:`e` is the elementary charge
- :math:`\rho` is the resistivity
- :math:`\vec{j}` is the current density

**Black's Equation:**

.. math::
   MTTF = A j^{-n} \exp\left(\frac{E_a}{k_B T}\right)

where:
- :math:`A` is a material constant
- :math:`j` is the current density
- :math:`n \approx 2` is the current exponent
- :math:`E_a` is the activation energy

Stress Analysis
---------------

**Stress-Strain Relationship:**

.. math::
   \sigma_{ij} = C_{ijkl} \epsilon_{kl}

For cubic crystals:

.. math::
   \begin{pmatrix}
   \sigma_{11} \\
   \sigma_{22} \\
   \sigma_{33} \\
   \sigma_{23} \\
   \sigma_{13} \\
   \sigma_{12}
   \end{pmatrix} = 
   \begin{pmatrix}
   C_{11} & C_{12} & C_{12} & 0 & 0 & 0 \\
   C_{12} & C_{11} & C_{12} & 0 & 0 & 0 \\
   C_{12} & C_{12} & C_{11} & 0 & 0 & 0 \\
   0 & 0 & 0 & C_{44} & 0 & 0 \\
   0 & 0 & 0 & 0 & C_{44} & 0 \\
   0 & 0 & 0 & 0 & 0 & C_{44}
   \end{pmatrix}
   \begin{pmatrix}
   \epsilon_{11} \\
   \epsilon_{22} \\
   \epsilon_{33} \\
   2\epsilon_{23} \\
   2\epsilon_{13} \\
   2\epsilon_{12}
   \end{pmatrix}

**Thermal Stress:**

.. math::
   \epsilon_{th} = \alpha_{th} \Delta T

**Piezoresistive Effect:**

.. math::
   \frac{\Delta \rho}{\rho} = \pi_l \sigma_l + \pi_t \sigma_t

where :math:`\pi_l` and :math:`\pi_t` are piezoresistive coefficients.

Multi-Physics Coupling
----------------------

**Electro-Thermal Coupling:**

.. math::
   \rho c_p \frac{\partial T}{\partial t} = \nabla \cdot (k \nabla T) + \sigma E^2

.. math::
   \nabla \cdot (\sigma \nabla \phi) = 0

where :math:`\phi` is the electric potential and :math:`\vec{E} = -\nabla \phi`.

**Thermo-Mechanical Coupling:**

.. math::
   \sigma_{ij} = C_{ijkl}(\epsilon_{kl} - \alpha_{th} \Delta T \delta_{kl})

**Electro-Mechanical Coupling:**

.. math::
   \sigma_{ij} = C_{ijkl} \epsilon_{kl} - e_{kij} E_k

where :math:`e_{kij}` is the piezoelectric tensor.

Numerical Implementation
-----------------------

**Finite Difference Discretization:**

.. math::
   \frac{\partial^2 u}{\partial x^2} \approx \frac{u_{i+1} - 2u_i + u_{i-1}}{(\Delta x)^2}

**Finite Element Weak Form:**

.. math::
   \int_\Omega \nabla u \cdot \nabla v \, d\Omega = \int_\Omega f v \, d\Omega + \int_{\Gamma_N} g v \, d\Gamma

**Time Integration (Crank-Nicolson):**

.. math::
   \frac{u^{n+1} - u^n}{\Delta t} = \frac{1}{2}[L(u^{n+1}) + L(u^n)]

where :math:`L` is the spatial operator.

These mathematical formulations provide the theoretical foundation for accurate and reliable semiconductor process simulation in SemiPRO.
