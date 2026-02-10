const mongoose = require('mongoose');

const PatientSchema = new mongoose.Schema({
  userId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'User',
    required: true
  },
  // Users (caregivers) who are allowed to manage this patient.
  // For now, we won't enforce this in queries yet – existing logic
  // still uses userId, so this field is additive and backwards compatible.
  caregivers: [
    {
      type: mongoose.Schema.Types.ObjectId,
      ref: 'User'
    }
  ],
  name: {
    type: String,
    required: true
  },
  age: Number,
  caregiverName: String,
  caregiverPhone: String,
  // Optional: email of the patient (used to link a patient User on signup)
  patientEmail: {
    type: String,
    lowercase: true,
    default: null
  },
  deviceId: {
    type: String,
    default: null
  },
  deviceActive: {
    type: Boolean,
    default: true
  },
  // Last heartbeat timestamp from ESP32 device
  deviceLastSeen: {
    type: Date,
    default: null
  }
});

module.exports = mongoose.model('Patient', PatientSchema);
