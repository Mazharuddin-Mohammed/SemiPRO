Semiconductor Physics Fundamentals
===================================

**Author: Dr. Mazharuddin Mohammed**

This section provides the theoretical foundation for the physics models implemented in SemiPRO.

Crystal Structure and Band Theory
---------------------------------

Silicon Crystal Structure
~~~~~~~~~~~~~~~~~~~~~~~~~

Silicon crystallizes in the diamond cubic structure with lattice parameter :math:`a = 5.431 \text{ Å}` at room temperature. The crystal structure can be described by the space group Fd3m with atoms at positions:

.. math::
   \vec{r}_i = \frac{a}{4}(0,0,0), \frac{a}{4}(1,1,0), \frac{a}{4}(1,0,1), \frac{a}{4}(0,1,1)

Band Structure
~~~~~~~~~~~~~~

The electronic band structure of silicon is characterized by:

- **Indirect bandgap**: :math:`E_g = 1.12 \text{ eV}` at 300K
- **Conduction band minima**: Located at 85% of the way from Γ to X points
- **Valence band maxima**: Located at the Γ point

The temperature dependence of the bandgap follows the Varshni equation:

.. math::
   E_g(T) = E_g(0) - \frac{\alpha T^2}{T + \beta}

where :math:`E_g(0) = 1.17 \text{ eV}`, :math:`\alpha = 4.73 \times 10^{-4} \text{ eV/K}`, and :math:`\beta = 636 \text{ K}`.

Carrier Statistics
------------------

Intrinsic Carrier Concentration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The intrinsic carrier concentration is given by:

.. math::
   n_i = \sqrt{N_c N_v} \exp\left(-\frac{E_g}{2k_B T}\right)

where:

- :math:`N_c = 2\left(\frac{2\pi m_e^* k_B T}{h^2}\right)^{3/2}` is the effective density of states in the conduction band
- :math:`N_v = 2\left(\frac{2\pi m_h^* k_B T}{h^2}\right)^{3/2}` is the effective density of states in the valence band
- :math:`m_e^* = 1.08 m_0` is the electron effective mass
- :math:`m_h^* = 0.56 m_0` is the hole effective mass

For silicon at 300K: :math:`n_i = 1.02 \times 10^{10} \text{ cm}^{-3}`

Doped Semiconductors
~~~~~~~~~~~~~~~~~~~

For a doped semiconductor with donor concentration :math:`N_D` and acceptor concentration :math:`N_A`, the charge neutrality equation is:

.. math::
   n + N_A^- = p + N_D^+

In the case of complete ionization and :math:`N_D \gg N_A`:

.. math::
   n \approx N_D - N_A = N_{net}

The minority carrier concentration is:

.. math::
   p = \frac{n_i^2}{n} = \frac{n_i^2}{N_{net}}

Transport Phenomena
-------------------

Drift and Diffusion
~~~~~~~~~~~~~~~~~~~

The current density in a semiconductor is described by the drift-diffusion equation:

.. math::
   \vec{J}_n = q n \mu_n \vec{E} + q D_n \nabla n

.. math::
   \vec{J}_p = q p \mu_p \vec{E} - q D_p \nabla p

where the diffusion coefficients are related to mobility by the Einstein relation:

.. math::
   D_n = \frac{k_B T}{q} \mu_n, \quad D_p = \frac{k_B T}{q} \mu_p

Mobility Models
~~~~~~~~~~~~~~

The mobility in silicon depends on temperature, doping concentration, and electric field. The low-field mobility is given by:

.. math::
   \mu_n(T, N) = \mu_{min,n} + \frac{\mu_{max,n} - \mu_{min,n}}{1 + \left(\frac{N}{N_{ref,n}}\right)^{\alpha_n}} \left(\frac{T}{300}\right)^{-\gamma_n}

For electrons in silicon:
- :math:`\mu_{max,n} = 1417 \text{ cm}^2/\text{V·s}`
- :math:`\mu_{min,n} = 68.5 \text{ cm}^2/\text{V·s}`
- :math:`N_{ref,n} = 9.68 \times 10^{16} \text{ cm}^{-3}`
- :math:`\alpha_n = 0.711`
- :math:`\gamma_n = 2.285`

High-field effects are modeled using the Caughey-Thomas model:

.. math::
   \mu(E) = \frac{\mu_0}{\left[1 + \left(\frac{\mu_0 E}{v_{sat}}\right)^{\beta}\right]^{1/\beta}}

where :math:`v_{sat} = 1.07 \times 10^7 \text{ cm/s}` is the saturation velocity and :math:`\beta = 2`.

Recombination Mechanisms
------------------------

Shockley-Read-Hall Recombination
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SRH recombination rate is given by:

.. math::
   R_{SRH} = \frac{np - n_i^2}{\tau_p(n + n_1) + \tau_n(p + p_1)}

where:

.. math::
   n_1 = n_i \exp\left(\frac{E_t - E_i}{k_B T}\right), \quad p_1 = n_i \exp\left(\frac{E_i - E_t}{k_B T}\right)

and :math:`E_t` is the trap energy level.

Auger Recombination
~~~~~~~~~~~~~~~~~~~

The Auger recombination rate is:

.. math::
   R_{Auger} = (C_n n + C_p p)(np - n_i^2)

For silicon:
- :math:`C_n = 2.8 \times 10^{-31} \text{ cm}^6/\text{s}`
- :math:`C_p = 9.9 \times 10^{-32} \text{ cm}^6/\text{s}`

Radiative Recombination
~~~~~~~~~~~~~~~~~~~~~~

For indirect bandgap semiconductors like silicon, radiative recombination is negligible:

.. math::
   R_{rad} = B(np - n_i^2)

where :math:`B \approx 10^{-15} \text{ cm}^3/\text{s}` for silicon.

Generation-Recombination Equations
----------------------------------

Continuity Equations
~~~~~~~~~~~~~~~~~~~~

The time evolution of carrier concentrations is governed by the continuity equations:

.. math::
   \frac{\partial n}{\partial t} = \frac{1}{q}\nabla \cdot \vec{J}_n + G_n - R_n

.. math::
   \frac{\partial p}{\partial t} = -\frac{1}{q}\nabla \cdot \vec{J}_p + G_p - R_p

where :math:`G` represents generation and :math:`R` represents recombination.

Poisson's Equation
~~~~~~~~~~~~~~~~~~

The electric potential is determined by Poisson's equation:

.. math::
   \nabla^2 \phi = -\frac{\rho}{\epsilon}

where the charge density is:

.. math::
   \rho = q(p - n + N_D^+ - N_A^-)

Thermal Physics
---------------

Heat Conduction
~~~~~~~~~~~~~~

Heat transport in semiconductors is governed by the heat conduction equation:

.. math::
   \rho c_p \frac{\partial T}{\partial t} = \nabla \cdot (k \nabla T) + H

where:
- :math:`\rho` is the density
- :math:`c_p` is the specific heat capacity
- :math:`k` is the thermal conductivity
- :math:`H` is the heat generation rate

For silicon:
- :math:`k = 150 \text{ W/m·K}` at 300K
- :math:`c_p = 700 \text{ J/kg·K}`
- :math:`\rho = 2330 \text{ kg/m}^3`

Joule Heating
~~~~~~~~~~~~

The heat generation due to current flow is:

.. math::
   H = \vec{J} \cdot \vec{E} = \frac{J^2}{\sigma}

where :math:`\sigma = q(n\mu_n + p\mu_p)` is the electrical conductivity.

Lattice Thermal Conductivity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The temperature dependence of thermal conductivity follows:

.. math::
   k(T) = k_0 \left(\frac{T}{T_0}\right)^{-\alpha}

where :math:`\alpha \approx 1.3` for silicon and :math:`k_0 = 150 \text{ W/m·K}` at :math:`T_0 = 300 \text{ K}`.

Mechanical Properties
--------------------

Stress and Strain
~~~~~~~~~~~~~~~~~

The stress-strain relationship for silicon (cubic crystal) is described by:

.. math::
   \sigma_{ij} = C_{ijkl} \epsilon_{kl}

For the cubic system, the elastic constants are:
- :math:`C_{11} = 165.7 \text{ GPa}`
- :math:`C_{12} = 63.9 \text{ GPa}`
- :math:`C_{44} = 79.6 \text{ GPa}`

Thermal Expansion
~~~~~~~~~~~~~~~~

The thermal strain is given by:

.. math::
   \epsilon_{th} = \alpha_{th} \Delta T

where :math:`\alpha_{th} = 2.6 \times 10^{-6} \text{ K}^{-1}` is the thermal expansion coefficient for silicon.

Piezoresistive Effect
~~~~~~~~~~~~~~~~~~~~

The change in resistivity due to mechanical stress is:

.. math::
   \frac{\Delta \rho}{\rho} = \pi_l \sigma_l + \pi_t \sigma_t

where :math:`\pi_l` and :math:`\pi_t` are the longitudinal and transverse piezoresistive coefficients.

Surface Physics
---------------

Surface States
~~~~~~~~~~~~~

The surface state density at the Si-SiO₂ interface is typically:

.. math::
   D_{it} \approx 10^{10} - 10^{11} \text{ cm}^{-2} \text{eV}^{-1}

Interface Charge
~~~~~~~~~~~~~~~

The total interface charge includes:
- Fixed oxide charge: :math:`Q_f \approx 10^{10} - 10^{11} \text{ cm}^{-2}`
- Mobile ionic charge: :math:`Q_m`
- Oxide trapped charge: :math:`Q_{ot}`
- Interface trapped charge: :math:`Q_{it}`

The flat-band voltage shift is:

.. math::
   \Delta V_{FB} = -\frac{Q_f + Q_m + Q_{ot} + Q_{it}}{C_{ox}}

This comprehensive physics foundation forms the basis for all process simulations in SemiPRO.
