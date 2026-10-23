# DSS keyword contract

The Data Subspace (DSS) keywords record which selections have been applied
to a data set. `dataSubselector::Cuts` is the only code in the Fermitools
that parses and writes them. Every tool that later needs to know "what cuts
were applied" (exposure, livetime cubes, IRF choice, response generation)
reads them back through `Cuts`. This file describes the format, how each
cut type is encoded, and how cut sets are combined and compared. It is the
part of `dataSubselector` that other packages depend on most, so a change
here is a cross-package change.

All statements are from reading `src/Cuts.cxx`, `src/CutBase.cxx` and the
cut classes at the commit in this package's `README.md`, unless marked
otherwise.

## Keywords

For each cut `n = 1 … NDSKEYS`, in the header of the HDU that carries the
selection (normally `EVENTS`; also the primary HDU of counts maps and
exposure maps, the `SPECTRUM` HDU of PHA files, `RATE` of light curves,
`EXPOSURE` of livetime cubes):

| Keyword | Meaning |
|---------|---------|
| `NDSKEYS` | Number of DSS cuts. **Required** when reading: `Cuts(file, ext)` calls `header["NDSKEYS"].get()` with no fallback, so an HDU without it throws (from reading the code; not checked at runtime). |
| `DSTYPn` | What is cut on: a column name, `COL[i]` for vector element `i` (1-based), `POS(RA,DEC)`, `BIT_MASK(...)`, or `<NAME>VERSION`. |
| `DSUNIn` | Unit string. |
| `DSVALn` | The selection value. |
| `DSREFn` | Only for table references (`DSVALn = TABLE`). |

`CutBase::writeDssKeywords` writes these with `tip`, one cut at a time.
`Cuts::writeDssKeywords(header)` first **erases** `DSTYP/DSUNI/DSVAL/DSREF`
1…old `NDSKEYS` and `NDSKEYS` itself, then renumbers from 1.

## Encoding per cut type

| Class (`type()`) | `DSTYPn` | `DSUNIn` | `DSVALn` | `DSREFn` |
|------------------|----------|----------|----------|----------|
| `RangeCut` (`"range"`) | `ENERGY`, `ZENITH_ANGLE`, `CALIB_VERSION[1]`, … | as given (`MeV`, `deg`, `s`, `dimensionless`) | `min:max`, `min:` (MINONLY) or `:max` (MAXONLY). Written with 20 significant digits. | — |
| `GtiCut` (`"GTI"`) | `TIME` | `s` | `TABLE` | `:GTI` |
| `SkyConeCut` (`"SkyCone"`) | `POS(RA,DEC)` | `deg` | `CIRCLE(ra,dec,radius)` (10 significant digits) | — |
| `BitMaskCut` (`"bit_mask"`), Pass 8 and later | `BIT_MASK(EVENT_CLASS,128,P8R3)` — the **mask** value, then `PASS_VER` | `DIMENSIONLESS` | `1:1` | — |
| `BitMaskCut`, pre-Pass 8 (`PASS_VER` ∈ `""`, `NONE`, `P6`, `P7V6`, `P7REP`, `P8R0`) | `BIT_MASK(EVENT_CLASS,2,P7REP)` — the **bit position** | `DIMENSIONLESS` | `1:1` | — |
| `VersionCut` (`"version"`) | `IRF_VERSION` | `DIMENSIONLESS` | IRF name, e.g. `P8R3_SOURCE_V3` | — |

Which bit-mask form is used is decided by `BitMaskCut::post_P7(pass_ver)`:
anything **not** in the hard-coded list above is treated as Pass 8 or
later. `P8R0` counts as old.

`VersionCut` is not a selection. `accept()` always returns `true` and
`filterString()` is empty. It exists only to carry the IRF name from
`gtselect`/`obsSim` into derived products.

## Reading: how a DSS triple becomes a cut

`Cuts(eventFile, extname, check_columns, skipTimeRangeCuts,
skipEventClassCuts)` tests each triple **in this order** (`DSVALn` is
upper-cased first; `DSTYPn` is not):

1. `DSVAL` starts with `CIRCLE` → `SkyConeCut`. `DSUNI` must start with
   `deg`. `DSTYP` may be `POS(RA,DEC)` or `POS(GLON,GLAT)`; galactic
   centres are converted to RA/Dec.
2. `DSTYP == TIME` and `DSVAL == TABLE` → `GtiCut(eventFile)`, which reads
   the extension named **`GTI`** of the same file. `DSREFn` is read but not
   used, so a GTI under another extension name is not found.
3. `DSTYP` starts with `BIT_MASK` → `BitMaskCut`. If the third argument is
   an old pass version, or absent, the number is treated as a bit position
   and shifted (`mask = 1 << n`).
4. `DSTYP` ends in `VERSION` → `VersionCut(DSTYP, DSVAL)`. Because `DSVAL`
   was upper-cased, the stored IRF name is upper case.
5. Otherwise, if `check_columns` is false **or** the column exists in the
   table, and `DSVAL != TABLE` → `RangeCut`, unless it is a `TIME` range
   and `skipTimeRangeCuts`, or an `EVENT_CLASS` range and
   `skipEventClassCuts` (the old, pre-bit-mask encoding).
6. Anything else throws `"FITS extension contains an unrecognized DSS
   filtering scheme"`. With `check_columns=true`, this includes a range cut
   on a column the table does not have.

`check_columns=true` dynamic-casts the HDU to a `tip::Table`. Use
`check_columns=false` for image HDUs (counts maps, exposure maps); every
consumer that reads images does so.

After parsing, the constructor sets `irfName()` from the first
`IRF_VERSION` cut (default `"NONE"`), and `pass_ver()` from the HDU's
`PASS_VER` keyword, falling back to the `EVENT_CLASS` bit-mask's pass
version, falling back to `"NONE"`.

## Writing: what goes where

| Method | Writes | Used by |
|--------|--------|---------|
| `writeDssKeywords(header)` | Every cut **except `TIME` range cuts**. The `GtiCut` is written (`TIME`/`TABLE`/`:GTI`). | `gtselect`, `obsSim`, `gtcntsmap`-style products in `Likelihood` (`CountsMapBase`, `gtexpcube2`, `expMap`, `meanPsf`, `gtaddlivetime`, `diffuseResponses`) |
| `writeDssTimeKeywords(header)` | **Only** time cuts: `GtiCut` and `TIME` range cuts. | `gtltcube` (`Likelihood/src/makeExposureCube`) via `RoiCuts` |
| `writeGtiExtension(file)` | The first `GtiCut`'s intervals as a `GTI` extension. | `Likelihood` (`ModelMap`, `gtaddlivetime`, `expMap`, `gtltcube`) |
| `writeCuts(stream, suppressGtis)` | A text listing in the same `DSTYPn: …` form, with GTI rows. | `gtvcut`; `Likelihood::AppHelpers::checkCuts` when `relyOnStreams` |

The consequence for event files: **a time selection is never recorded as a
`TIME` range DSS keyword by `gtselect`**. It is recorded only by trimming
the GTI extension. A `TIME` range in an old file is dropped on read by
`gtselect` (`skipTimeRangeCuts=true`).

## Adding a cut to a set (`Cuts::addCut`)

The private `addCut(CutBase*)` behind every `addXxxCut` method:

1. If an **equal** cut is already present, the new one is discarded.
2. Otherwise, for each existing cut, in order: if the new cut
   `supercedes` it, the new cut **replaces** it in place; if the existing
   one supercedes the new cut, the new cut is discarded.
3. Otherwise the new cut is appended.

The public `addCut(const CutBase&)` overload **skips all of this** and
appends a clone, so duplicates and conflicts are possible through it.

`supercedes` rules (the new cut is `this`):

| Type | New cut supersedes the existing one when |
|------|------------------------------------------|
| `RangeCut` | Same `colname` **and** same interval type, and the new range is inside the existing range (`old.min ≤ new.min` and `new.max ≤ old.max`). Ranges of different types, or partly overlapping ranges, are both kept. |
| `SkyConeCut` | The new cone lies entirely inside the old one (`r_new ≤ r_old − separation`). |
| `BitMaskCut` `EVENT_TYPE` | Always, **if** the new mask is valid against the current mask per the CALDB `valid_evtype_selections.txt`; otherwise it **throws** "The requested evtype selection … is inconsistent with the existing evtype selection". |
| `BitMaskCut` `EVENT_CLASS`, Pass 8+ | Always, if valid per `valid_evclass_selections.txt` (with a chatter-3 warning when the mask changes); otherwise throws. |
| `BitMaskCut` `EVENT_CLASS`, pre-Pass 8 | When the new mask is numerically larger (assumes classes are nested). |
| `VersionCut` | Always, when the name (`IRF_VERSION`) matches: the newer version replaces the older. |
| `GtiCut` | Never. Two different GTIs both remain; their effect is the intersection. |

The validity files map "current mask" → "allowed bits" (lines
`current,allowed`); a new mask is valid when `allowed & new == new`. If the
current mask is not listed, `validMask` throws `"Current bit mask, N, not
found in validity mask file."`.

`Cuts::mergeRangeCuts()` then collapses all `RangeCut`s on the same column
into one, taking the largest lower bound and smallest upper bound. If the
ranges are disjoint, the result has `min > max` and accepts nothing; no
error is raised. `gtselect` always calls it, so repeated `gtselect` runs
can only narrow a range.

## Comparing cut sets

| Function | Rule |
|----------|------|
| `Cuts::operator==` | Same number of cuts, and every cut in `rhs` is equal to *some* cut in `this`. **Order-insensitive**, despite the header comment saying ordering must match. GTIs compared interval by interval (`evtbin::Gti::operator==`, exact doubles). |
| `Cuts::compareWithoutGtis` | Same number of cuts (**GTIs still count**), and every non-GTI cut in `rhs` is found in `this`. So a file with a GTI cut and a file without one never match. |
| `CutBase::operator==` | Same `type()` and the subclass `equals()`: exact `double` comparison for ranges and cones; for `BitMaskCut`, column and mask only (`pass_ver` is **ignored**). `RangeCut` compares only the bound(s) that its interval type uses. |
| Likelihood `AppHelpers::checkCuts(..., relyOnStreams=true)` | Compares the `writeCuts` text, which is **order-sensitive**. |

Because `RangeCut` values are written with 20 digits and cones with 10, a
cut round-trips through a FITS header to an equal value, so equality across
files written by the same code is reliable. Hand-edited or
differently-formatted DSS values may not compare equal.

## Multiple input files

`Cuts(std::vector<std::string> files, ext, ...)` reads each file, requires
`compareWithoutGtis` against the **first** file (else throws `"DSS keywords
in X do not match those in Y"`), then calls `Cuts::mergeGtis`: all non-GTI
cuts are taken from the first file and the GTIs from all files are
**unioned** into a single `GtiCut`.

`irfName()` is then set from the first file. `pass_ver()` is **not** set
(it is the empty string, not `"NONE"`), because `operator=` does not copy
it and the vector constructor never calls `read_pass_ver` (from reading
the code; not checked at runtime).

## IRF inference from DSS (`CALDB_implied_irfs`)

`Likelihood::AppHelpers::createResponseFuncs` and
`pyLikelihood`'s `UnbinnedAnalysis`/`BinnedAnalysis` call
`Cuts::CALDB_implied_irfs()` when `irfs=CALDB`:

1. Read `$CALDB/data/glast/lat/bcf/irf_index.fits[BITMASK_MAPPING]` into
   name → `1 << bitposition`.
2. Take the `EVENT_CLASS` bit-mask cut (throw if there is none).
3. If `IRF_VERSION` is `NONE`, pick the IRF with the same mask and the same
   `pass_ver()` and the highest `Vn`. Otherwise use the named IRF and warn
   if a newer version exists for the same mask.
4. Append the event-type partition name (`" (PSF)"`, `" (EDISP)"`) from the
   `EVENT_TYPE` mask, via `irfUtil::EventTypeMapper` (also reads
   `irf_index.fits`). The bit used is `floor(log2(mask))`, i.e. the
   **highest** set bit.

`Cuts::setIrfs(name)` goes the other way: from an IRF name (optionally with
`::FRONT`/`::BACK`) it adds the `EVENT_CLASS` bit-mask cut, a
`CONVERSION_TYPE` range cut if a section was given, and an `IRF_VERSION`
version cut. `obsSim` and `AppHelpers` (for `irfs≠CALDB`) use it.

## Who reads and writes DSS keywords

| Package | Reads | Writes |
|---------|-------|--------|
| `dataSubselector` | `gtselect`, `gtmktime` (zenith/cone for `roicut`, GTI), `gtvcut` | `gtselect` |
| `Likelihood` | `RoiCuts::readCuts`, `AppHelpers::checkCuts`/`checkTimeCuts` (FT1 vs. livetime cube, exposure map, counts map), IRF inference | counts maps, exposure maps, `gtltcube` (time cuts only), `gtaddlivetime`, `gtpsf`, `gtdiffrsp` |
| `observationSim` | — | simulated FT1 (`EventContainer`) |
| `evtbin` | Copies DSS keywords from input to its products directly (not through `Cuts`). See `src/evtbin/doc/architecture/`. | — |
| `rspgen` | `SPECTRUM` header: the `SkyConeCut` gives the PSF integration radius | — |
| `pyExposure` (`gtexposure`) | `RATE` header of a light curve: cone, energy range | — |
| `SolarSystemTools` | counts maps and sun livetime cubes | `gtexpcubesun`, `gtexphpsun` (`writeDssKeywords`); sun livetime cubes (`writeDssTimeKeywords`, `writeGtiExtension`) |
| `pyLikelihood` | `pyLike.Cuts(evfile, "EVENTS", False, True, True)` for IRF inference | — |
