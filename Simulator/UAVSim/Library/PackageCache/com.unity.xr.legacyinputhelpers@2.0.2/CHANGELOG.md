# Changelog
All notable changes to this package will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [2.0.2] - 2019-03-12
fixes compiler error on non XR platforms (thanks @zilys!)

## [2.0.1] - 2019-01-02
fixes standalone compile error / forward port of 1.3.2 fix

## [2.0.0] - 2019-01-02
breaking changes to API to allow pose data queries to indiciate what data was actually valid. this fixes the bug where position was being set to identity if only rotation was provided by the input system.

Pose provider API has now changed to return a PoseDataFlags bitflag, the bitflag will indiciate what pieces of data was set on the output pose parameter.
```csharp
public abstract bool TryGetPoseFromProvider(out Pose output)
```
is now
```csharp
public virtual PoseDataFlags GetPoseFromProvider(out Pose output)
```
All pose providers in this package have been updated, as has the tracked pose driver code to correctly handle the returned bitflags. Any user derived users of this API will need to also update their code accordingly.

New unit tests added for this case in the tracked pose driver

## [1.3.2] - 2019-01-02
fixes standalone compile error

## [1.3.1] - 2018-12-17
merges 1.0.3 into mainline branch.

## [1.3.0] - 2018-12-05
Adds arm model support for 3dof controllers.

## [1.2.1] - 2018-10-25
Makes the input settings menu pop up when you click the menu item

## [1.2.0] - 2018-10-15
Adds the XR Binding Input Asset Seeder and associated documentation and tests

## [1.1.0] - 2018-10-11
Moved some classes internal for cleaner docs.

## [1.0.3] - 2018-11-09
fixes enums erroneously removed

## [1.0.2] - 2018-10-25
fix for inconsistent line endings in trackedposedriver.cs

## [1.0.1] - 2018-10-10
ported API documentation from the engine

## [1.0.0] - 2018-10-09
release prep

## [0.0.4] - 2018-10-09
ci fixes

## [0.0.3] - 2018-10-08
fix for changelog values.

## [0.0.2] - 2018-10-08
updated to latest upm package template

## [0.0.1] - 2018-10-08

### This is the first release of *Unity Package XR Tools*
Initial move from XR Tools to Legacy Input Helper package

