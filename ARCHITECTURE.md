ARCHITECTURE.md — Oyster
> Adap8ve Travel Memory System ---
## 1. Tech Stack & Jus8fica8on
|Layer|Choice|Why|
|---|---|---|
| **Framework** | React (Vite) | Component model maps cleanly to the step-by-step wizard UI; fast dev server; vibe coding friendly |
| **Map** | `react-simple-maps` | Declara8ve SVG map — each country is a addressable React component; no 8le server needed |
| **Styling** | Tailwind CSS | U8lity-first; great for rapid itera8on with AI-assisted coding |
| **Storage** | IndexedDB via `idb` | Handles binary photo blobs; persists across sessions; no backend required |
| **EXIF** | `exifr` | Lightweight client-side GPS + 8mestamp extrac8on from photos |
| **Voice** | Web Speech API | Na8ve browser API; zero dependencies; no API key |
| **Fonts** | Google Fonts (`Caveat` + `Kalam`) | Free handwri8ng fonts with decent La8n + CJK coverage |
| **Anima8on** | CSS keyframes + Framer Mo8on | CSS for Polaroid develop; Framer Mo8on for page transi8ons |
| **Build/Deploy** | Vite + GitHub Pages | Zero-config sta8c deploy; fits a no-backend MVP |
---
## 2. Data Model
All data lives in the browser. Two IndexedDB stores: `trips` and `sefngs`. ### `trips` store
Each document represents one trip entry. ```ts
Trip{
id: string country: string city: string startDate: string endDate: string photos: Photo[] createdAt: number updatedAt: number
// uuid, primary key
// ISO 3166-1 alpha-2 country code, e.g. "JP"
// freetext, e.g. "Kyoto" //ISOdate"2025-03-10" // ISO date "2025-03-15"
// ordered array, max 6 //Unix8mestamp
//Unix8mestamp
}
Photo {
id: string // uuid
blob:
cap8on:
takenAt:
lat: number|null // from EXIF GPS lng: number|null // from EXIF GPS
} ```
### `sefngs` store
Single document keyed `"user"`. ```ts
Sefngs {
soundEnabled: boolean //Polaroidshurersoundon/off reducedMo8on:boolean //mirrorsprefers-reduced-mo8on lastViewedAt: number // Unix 8mestamp, for new-memory badge logic
} ```
### Derived state (in-memory only)
```ts
// computed from trips store on app load
unlockedCountries:Set<string> //ISOcodesofcountrieswith≥1trip newMemoryBadges: Set<string> //countriesupdatedauerlastViewedAt ```
---
Blob string
// raw image stored in IndexedDB // max 140 chars
string|null // from EXIF, ISO date8me
## 3. Applica8on Architecture ```
src/
├── main.jsx
├── store/
│ ├──db.js
│ └──useTrips.js ├── pages/
│ ├──Home.jsx │ ├──Record.jsx │ └──Diary.jsx
# App entry, router setup #idbwrapper—open,read,writetrips+sefngs
#Reacthook—CRUDopera8ons+derivedstate
#Worldmapview(S02)
#Recordingwizardshell(S03–S07) #Singletripdiaryview(S09)

├── components/
│ ├──map/
│ │ ├── WorldMap.jsx # react-simple-maps wrapper
│ │ └── CountryLayer.jsx # per-country fill + interac8on logic
│ ├──record/
│ │ ├── BoardingPass.jsx # Step 1 — des8na8on + date input
│ │ ├──PhotoUpload.jsx #Step2—gallerypicker+EXIFread
│ │ ├──PolaroidReveal.jsx#Step3—developanima8on
│ │ ├──Cap8onSlider.jsx#Step4—per-photocap8on+voice
│ │ └── DiaryPreview.jsx # Step 5 — preview before save
│ ├──diary/
│ │ ├── DiaryPage.jsx # Journal page layout
│ │ └── PolaroidCard.jsx # Individual photo card component
│ └──ui/
│ ├── VoiceInput.jsx # Web Speech API wrapper
│ └── UnlockAnima8on.jsx # Map unlock overlay └── assets/
└── ci8es.json # Sta8c dataset — top 5,000 ci8es for autocomplete ```
---
## 4. Key Interac8on Flows
### Recording a new memory ```
Home (map)
→ tap CTA
→ Record wizard mounts
[1]BoardingPass —writesdes8na8on+datestodraustate
[2] PhotoUpload — reads blobs + EXIF, appends to drau
[3] PolaroidReveal — anima8on only, no state change
[4] Cap8onSlider — writes cap8ons to drau photo array [5]DiaryPreview —rendersdrau;onsave:commitstoIndexedDB
→ Home re-renders with new country unlocked ```
### Viewing a memory ```
Home (map)
→ tap unlocked country

→ Diary mounts, reads trips filtered by country code
→ if mul8ple trips: swipe between entries
→ tap Edit → returns to Cap8onSlider with exis8ng data pre-filled
```
---
## 5. Agen8c Engineering Plan
This project is built using AI-assisted vibe coding. The following plan structures how AI agents are prompted and sequenced to build each milestone without accumula8ng technical debt.
### Principles
- **One component per prompt.** Each AI session targets a single component or feature. Mixing concerns in one prompt leads to tangled, hard-to-debug output.
- **State before UI.** Define the data model and `useTrips` hook first. All components consume from this single source of truth — prevents props-drilling chaos later.
- **Prompt with constraints.** Every component prompt includes: the component's file path, its props interface, and what it must NOT do (e.g. "do not manage your own fetch logic").
- **Test in isola8on.** Each component is rendered standalone with mock data before being wired into the wizard flow.
### Prompt Sequence by Milestone
**M1 — Sta8c UI**
1. Prompt: `db.js` + `useTrips.js` hook with full CRUD interface (no UI yet)
2. Prompt: `WorldMap.jsx` + `CountryLayer.jsx` with hardcoded unlocked countries for visual tes8ng
3. Prompt: `BoardingPass.jsx` sta8c layout (no submit logic)
4. Prompt: `PhotoUpload.jsx` with drag-and-drop and thumbnail preview
**M2 — Anima8on + Input**
5. Prompt: `PolaroidReveal.jsx` CSS keyframe anima8on, accepts photo array as props 6. Prompt: `Cap8onSlider.jsx` with swipe naviga8on and text input
7. Prompt: `VoiceInput.jsx` Web Speech API wrapper with fallback state
8. Prompt: Wire Steps 1–4 into `Record.jsx` wizard shell with shared drau state
**M3 — Save + Map Unlock**
9. Prompt: `DiaryPreview.jsx` + save ac8on that calls `useTrips` write
10. Prompt: `UnlockAnima8on.jsx` overlay triggered auer save
11. Prompt: `CountryLayer.jsx` update — read unlock state from `useTrips`

**M4 — Diary View + Edit**
12. Prompt: `DiaryPage.jsx` + `PolaroidCard.jsx` reading from IndexedDB 13. Prompt: Edit mode — pre-fill `Cap8onSlider` with exis8ng trip data
**M5 — Discovery Feature**
14. Prompt: Mood + ac8vity filter UI, sta8c recommenda8on logic
15. Prompt: Highlight suggested countries as a separate layer on `WorldMap`
### Handling AI-Generated Code
- Auer each AI session, manually review for: hardcoded state, inline styles that conflict with Tailwind, and missing null checks on EXIF data.
- Keep a `CHANGELOG.md` no8ng what each session produced and what was manually patched. - If a component exceeds ~150 lines, prompt AI to split it before con8nuing.
---
*Oyster · ARCHITECTURE.md v1.0