# ARCHITECTURE.md — Oyster

> Adaptive Travel Memory System · 2026

---

## 1. Tech Stack & Justification

| Layer | Choice | Why |
|---|---|---|
| **Framework** | React (Vite) | Component model maps cleanly to the step-by-step wizard UI; fast dev server; vibe coding friendly |
| **Map** | `react-simple-maps` | Declarative SVG map — each country is an addressable React component; no tile server needed |
| **Styling** | Tailwind CSS | Utility-first; great for rapid iteration with AI-assisted coding |
| **Storage** | IndexedDB via `idb` | Handles binary photo blobs; persists across sessions; no backend required |
| **EXIF** | `exifr` | Lightweight client-side GPS + timestamp extraction from photos |
| **Voice** | Web Speech API | Native browser API; zero dependencies; no API key |
| **Fonts** | Google Fonts (`Caveat` + `Kalam`) | Free handwriting fonts with decent Latin + CJK coverage |
| **Animation** | CSS keyframes + Framer Motion | CSS for Polaroid develop; Framer Motion for page transitions |
| **Build/Deploy** | Vite + GitHub Pages | Zero-config static deploy; fits a no-backend MVP |

---

## 2. Data Model

All data lives in the browser. Two IndexedDB stores: `trips` and `settings`.

### `trips` store

Each document represents one trip entry.

```ts
Trip {
  id:          string       // uuid, primary key
  country:     string       // ISO 3166-1 alpha-2 country code, e.g. "JP"
  city:        string       // freetext, e.g. "Kyoto"
  startDate:   string       // ISO date "2025-03-10"
  endDate:     string       // ISO date "2025-03-15"
  photos:      Photo[]      // ordered array, max 6
  createdAt:   number       // Unix timestamp
  updatedAt:   number       // Unix timestamp
}

Photo {
  id:          string       // uuid
  blob:        Blob         // raw image stored in IndexedDB
  caption:     string       // max 140 chars
  takenAt:     string|null  // from EXIF, ISO datetime
  lat:         number|null  // from EXIF GPS
  lng:         number|null  // from EXIF GPS
}
```

### `settings` store

Single document keyed `"user"`.

```ts
Settings {
  soundEnabled:   boolean   // Polaroid shutter sound on/off
  reducedMotion:  boolean   // mirrors prefers-reduced-motion
  lastViewedAt:   number    // Unix timestamp, for new-memory badge logic
}
```

### Derived state (in-memory only)

```ts
// computed from trips store on app load
unlockedCountries: Set<string>   // ISO codes of countries with ≥1 trip
newMemoryBadges:   Set<string>   // countries updated after lastViewedAt
```

---

## 3. Application Architecture

```
src/
├── main.jsx                   # App entry, router setup
├── store/
│   ├── db.js                  # idb wrapper — open, read, write trips + settings
│   └── useTrips.js            # React hook — CRUD operations + derived state
├── pages/
│   ├── Home.jsx               # World map view
│   ├── Record.jsx             # Recording wizard shell
│   └── Diary.jsx              # Single trip diary view
├── components/
│   ├── map/
│   │   ├── WorldMap.jsx       # react-simple-maps wrapper
│   │   └── CountryLayer.jsx   # per-country fill + interaction logic
│   ├── record/
│   │   ├── BoardingPass.jsx   # Step 1 — destination + date input
│   │   ├── PhotoUpload.jsx    # Step 2 — gallery picker + EXIF read
│   │   ├── PolaroidReveal.jsx # Step 3 — develop animation
│   │   ├── CaptionSlider.jsx  # Step 4 — per-photo caption + voice
│   │   └── DiaryPreview.jsx   # Step 5 — preview before save
│   ├── diary/
│   │   ├── DiaryPage.jsx      # Journal page layout
│   │   └── PolaroidCard.jsx   # Individual photo card component
│   └── ui/
│       ├── VoiceInput.jsx     # Web Speech API wrapper
│       └── UnlockAnimation.jsx# Map unlock overlay
└── assets/
    └── cities.json            # Static dataset — top 5,000 cities for autocomplete
```

---

## 4. Key Interaction Flows

### Recording a new memory
```
Home (map)
  → tap CTA
  → Record wizard mounts
      [1] BoardingPass    — writes destination + dates to draft state
      [2] PhotoUpload     — reads blobs + EXIF, appends to draft
      [3] PolaroidReveal  — animation only, no state change
      [4] CaptionSlider   — writes captions to draft photo array
      [5] DiaryPreview    — renders draft; on save: commits to IndexedDB
  → Home re-renders with new country unlocked
```

### Viewing a memory
```
Home (map)
  → tap unlocked country
  → Diary mounts, reads trips filtered by country code
  → if multiple trips: swipe between entries
  → tap Edit → returns to CaptionSlider with existing data pre-filled
```

---

## 5. Agentic Engineering Plan

This project is built using AI-assisted vibe coding. The following plan structures how AI agents are prompted and sequenced to build each milestone without accumulating technical debt.

### Principles

- **One component per prompt.** Each AI session targets a single component or feature. Mixing concerns in one prompt leads to tangled, hard-to-debug output.
- **State before UI.** Define the data model and `useTrips` hook first. All components consume from this single source of truth — prevents props-drilling chaos later.
- **Prompt with constraints.** Every component prompt includes: the component's file path, its props interface, and what it must NOT do (e.g. "do not manage your own fetch logic").
- **Test in isolation.** Each component is rendered standalone with mock data before being wired into the wizard flow.

### Prompt Sequence by Milestone

**M1 — Static UI**
1. Prompt: `db.js` + `useTrips.js` hook with full CRUD interface (no UI yet)
2. Prompt: `WorldMap.jsx` + `CountryLayer.jsx` with hardcoded unlocked countries for visual testing
3. Prompt: `BoardingPass.jsx` static layout (no submit logic)
4. Prompt: `PhotoUpload.jsx` with drag-and-drop and thumbnail preview

**M2 — Animation + Input**
5. Prompt: `PolaroidReveal.jsx` CSS keyframe animation, accepts photo array as props
6. Prompt: `CaptionSlider.jsx` with swipe navigation and text input
7. Prompt: `VoiceInput.jsx` Web Speech API wrapper with fallback state
8. Prompt: Wire Steps 1–4 into `Record.jsx` wizard shell with shared draft state

**M3 — Save + Map Unlock**
9. Prompt: `DiaryPreview.jsx` + save action that calls `useTrips` write
10. Prompt: `UnlockAnimation.jsx` overlay triggered after save
11. Prompt: `CountryLayer.jsx` update — read unlock state from `useTrips`

**M4 — Diary View + Edit**
12. Prompt: `DiaryPage.jsx` + `PolaroidCard.jsx` reading from IndexedDB
13. Prompt: Edit mode — pre-fill `CaptionSlider` with existing trip data

**M5 — Discovery Feature**
14. Prompt: Mood + activity filter UI, static recommendation logic
15. Prompt: Highlight suggested countries as a separate layer on `WorldMap`

### Handling AI-Generated Code

- After each AI session, manually review for: hardcoded state, inline styles that conflict with Tailwind, and missing null checks on EXIF data.
- Keep a `CHANGELOG.md` noting what each session produced and what was manually patched.
- If a component exceeds ~150 lines, prompt AI to split it before continuing.

---

*Oyster · ARCHITECTURE.md v1.0 · 2026*
