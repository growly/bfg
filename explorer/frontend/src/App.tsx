import React from 'react'
import TwoPaneLayout from './components/Layout/TwoPaneLayout'

function App() {
  return (
    <div className="app">
      <header style={{ padding: '1rem', borderBottom: '1px solid #ccc' }}>
        <h1 style={{ margin: 0 }}>BFG Explorer</h1>
      </header>
      <TwoPaneLayout />
    </div>
  )
}

export default App
