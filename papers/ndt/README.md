# NDT

## Paper
- Peter Biber and Wolfgang Strasser, IROS 2003
- Martin Magnusson, PhD thesis 2009 for the 3D formulation

## What This Repository Implements

This is a compact NDT-style registration module:

- the target cloud is converted into voxel Gaussians
- the source cloud is aligned against those Gaussians
- the optimizer is intentionally lightweight and does not aim to be a full Magnusson-style production NDT port

## Current Scope

- voxel Gaussian map construction
- analytic gradient and Gauss-Newton Hessian approximation
- line search on the update step

## Not Included Yet

- full production-grade NDT optimizer behavior as found in mature libraries
- neighbor-cell interpolation and other accuracy-focused refinements
- an exact port of Autoware or PCL NDT internals
