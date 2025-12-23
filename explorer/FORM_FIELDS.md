# BFG Explorer - Form Fields Documentation

The BFG Explorer now has proper form fields for all 7 generators instead of raw JSON editing!

## Features

### ✅ Implemented

1. **Dynamic Form Generation** - Forms are automatically generated from parameter schemas
2. **Field Types**:
   - Number inputs with units (nm)
   - Checkboxes for booleans
   - Dropdowns for enums (CompassDirection)
   - Array fields for sequences (Sky130TransmissionGateStack)
3. **Load Example Button** - Quickly load example parameters
4. **Field Validation** - Min/max values, step increments
5. **Help Icons** - Hover tooltips with field descriptions
6. **Visual Feedback** - Clear labels, units, and descriptions

## Generator Forms

### 1. Sky130 Decap
**Fields:**
- Width (nm) - number input
- Height (nm) - number input
- NFET Width (nm) - number input (optional)
- PFET Width (nm) - number input (optional)
- NFET Length (nm) - number input (optional)
- PFET Length (nm) - number input (optional)
- Label Pins - checkbox
- Draw Overflowing Vias/Pins - checkbox

**Example Values:**
- Width: 1380 nm
- Height: 2720 nm
- Label Pins: ✓ Enabled
- Draw Overflowing Vias/Pins: ✓ Enabled

---

### 2. Sky130 Transmission Gate
**Fields:**
- PMOS Width (nm) - number input
- PMOS Length (nm) - number input
- NMOS Width (nm) - number input
- NMOS Length (nm) - number input
- Stacks Left - checkbox
- Stacks Right - checkbox
- Vertical Tab Pitch (nm) - number input
- Vertical Tab Offset (nm) - number input
- Poly Pitch (nm) - number input
- PMOS Tab Position - dropdown (8 compass directions)
- NMOS Tab Position - dropdown (8 compass directions)
- Draw N-Well - checkbox

**Example Values:**
- PMOS Width: 1000 nm
- PMOS Length: 150 nm
- NMOS Width: 650 nm
- NMOS Length: 150 nm
- Poly Pitch: 500 nm

---

### 3. Sky130 Transmission Gate Stack
**Fields:**
- Gate Sequences - array (Add/Remove buttons)
  - Each sequence has: Nets (comma-separated string, e.g., "A,B,C")
- PMOS Width (nm) - number input
- PMOS Length (nm) - number input
- NMOS Width (nm) - number input
- NMOS Length (nm) - number input
- Local Interconnect Width (nm) - number input (optional)
- Minimum Height (nm) - number input
- Poly Pitch (nm) - number input
- Horizontal Pitch (nm) - number input
- Insert Dummy Poly - checkbox
- Expand Wells Vertically - checkbox

**Example Values:**
- Sequences: [{ nets: "A,B,C" }]
- PMOS Width: 1000 nm
- Minimum Height: 2720 nm

**Array Field UI:**
```
┌─────────────────────────────────┐
│ Sequence 1              [Remove]│
│ ┌─────────────────────────────┐ │
│ │ Nets: A,B,C                 │ │
│ └─────────────────────────────┘ │
└─────────────────────────────────┘
        [+ Add Sequence]
```

---

### 4. Sky130 Interconnect Mux1
**Fields:**
- Number of Inputs - number input (1-32)
- Number of Outputs - number input (1-8)
- Poly Pitch (nm) - number input
- Vertical Pitch (nm) - number input
- Vertical Offset (nm) - number input
- Horizontal Pitch (nm) - number input
- Vertical Routing Channel Width (nm) - number input (optional)
- Power/Ground Strap Width (nm) - number input

**Example Values:**
- Inputs: 6
- Outputs: 1
- Poly Pitch: 500 nm
- Vertical Pitch: 340 nm

---

### 5. LUT-B
**Fields:**
- LUT Size (K) - number input (2-6)

**Example Values:**
- LUT Size: 4

---

### 6. Interconnect
**Fields:**
- Number of Rows - number input (1-64)
- Number of Columns - number input (1-64)

**Example Values:**
- Rows: 8
- Columns: 16

---

### 7. Slice
**No configurable parameters**
- Message: "This generator has no configurable parameters."
- Only "Generate" button is shown

---

## UI Layout

```
┌─────────────────────────────────────────────────────┐
│ BFG Explorer                                        │
├──────────────────┬──────────────────────────────────┤
│ Generator:       │                                  │
│ [Sky130 Decap ▼] │                                  │
│                  │       Graphic | Proto Text       │
│ Description:     │  ┌─────────────────────────────┐ │
│ Decoupling...    │  │                             │ │
│                  │  │    [SVG Visualization]      │ │
│ Width            │  │                             │ │
│ [1380      ] nm  │  │                             │ │
│                  │  │                             │ │
│ Height           │  │                             │ │
│ [2720      ] nm  │  └─────────────────────────────┘ │
│                  │                                  │
│ Label Pins       │                                  │
│ ☑ Enabled        │                                  │
│                  │                                  │
│ [Load Example]   │                                  │
│   [Generate]     │                                  │
└──────────────────┴──────────────────────────────────┘
```

## Form Validation

- **Number fields**: Min/max validation, step increments (usually 10 nm)
- **Required fields**: None (all fields are optional per proto spec)
- **Empty values**: Removed from parameters (not sent to backend)
- **Invalid values**: Browser validates number ranges

## Visual Elements

### Field with Unit
```
Width
[1380      ] nm
  ↑          ↑
 input     unit label
```

### Checkbox
```
☑ Label Pins
Enabled
```

### Dropdown (Enum)
```
PMOS Tab Position
[NORTH     ▼]
```

### Help Icon
```
Width ?
      ↑
   Hover for description
```

### Array Field
```
┌─ Gate Sequences ────────────────┐
│ Sequence 1              [Remove]│
│ Nets: A,B,C                     │
├─────────────────────────────────┤
│ Sequence 2              [Remove]│
│ Nets: D,E,F                     │
└─────────────────────────────────┘
        [+ Add Sequence]
```

## Color Scheme

- Primary button (Generate): Blue (#007bff)
- Secondary button (Load Example): Gray (#6c757d)
- Delete button (Remove): Red (#dc3545)
- Add button: Green (#28a745)
- Help icon: Blue circle
- Description box: Light blue background

## Accessibility

- All form fields have proper labels
- Tab navigation works correctly
- Help text available via title attributes
- Clear visual feedback on focus
- Color contrast meets WCAG guidelines

## Usage Tips

1. **Start with Load Example** - Click "Load Example" to populate default values
2. **Modify as Needed** - Change any fields you want to customize
3. **Optional Fields** - Leave blank to use BFG defaults
4. **Arrays** - Use Add/Remove buttons for sequences
5. **Generate** - Click Generate to create the layout

The form automatically updates when you change generators!
