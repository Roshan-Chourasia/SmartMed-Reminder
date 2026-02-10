// Device Management Functions

function openLinkDeviceModal(patientId) {
  if (!Auth.isAuthenticated()) {
    Auth.redirectToLogin();
    return;
  }
  const modal = document.getElementById('linkDeviceModal');
  const patientIdInput = document.getElementById('linkPatientId');
  if (modal && patientIdInput) {
    patientIdInput.value = patientId;
    modal.style.display = 'flex';
  }
}

function closeLinkDeviceModal() {
  const modal = document.getElementById('linkDeviceModal');
  if (modal) {
    modal.style.display = 'none';
    document.getElementById('linkDeviceForm').reset();
  }
}

async function handleLinkDevice(event) {
  event.preventDefault();
  
  try {
    const patientId = document.getElementById('linkPatientId').value;
    const deviceId = document.getElementById('deviceIdInput').value.trim();

    if (!patientId || !deviceId) {
      showToast('Patient ID and Device ID are required', 'error');
      return;
    }

    await Auth.apiFetch('/api/device/link', {
      method: 'POST',
      body: { patientId, deviceId }
    });

    showToast('Device linked successfully', 'success');
    closeLinkDeviceModal();
    await loadPatients(); // Reload to show updated device status
  } catch (err) {
    if (err.status === 401) {
      Auth.redirectToLogin();
    } else {
      showToast('Failed to link device: ' + (err.message || 'Unknown error'), 'error');
    }
  }
}

async function handleDeviceAction(patientId, action) {
  try {
    let endpoint = '';
    let successMessage = '';

    switch (action) {
      case 'enable':
        endpoint = '/api/device/enable';
        successMessage = 'Device enabled successfully';
        break;
      case 'disable':
        endpoint = '/api/device/disable';
        successMessage = 'Device disabled successfully';
        break;
      case 'unlink':
        if (!confirm('Are you sure you want to unlink this device? The patient will need to link a device again to use the system.')) {
          return;
        }
        endpoint = '/api/device/unlink';
        successMessage = 'Device unlinked successfully';
        break;
      default:
        showToast('Invalid action', 'error');
        return;
    }

    await Auth.apiFetch(endpoint, {
      method: 'POST',
      body: { patientId }
    });

    showToast(successMessage, 'success');
    await loadPatients(); // Reload to show updated status
  } catch (err) {
    if (err.status === 401) {
      Auth.redirectToLogin();
    } else {
      showToast('Failed to ' + action + ' device: ' + (err.message || 'Unknown error'), 'error');
    }
  }
}

// Get active device ID from patients (for use in schedule/logs)
function getActiveDeviceId() {
  // If a device is explicitly selected (for caregivers), use that
  if (window.selectedDeviceId) {
    // Verify the selected device still exists and is active
    const selectedPatient = window.allPatients && window.allPatients.find(p => p.deviceId === window.selectedDeviceId && p.deviceActive);
    if (selectedPatient) {
      return window.selectedDeviceId;
    }
    // If selected device is no longer valid, clear selection
    window.selectedDeviceId = null;
    if (window.deviceSelector) {
      window.deviceSelector.value = '';
    }
  }
  
  // Fallback: find first active device (for patients or when no selection)
  if (typeof window.allPatients === 'undefined' || !window.allPatients || window.allPatients.length === 0) {
    return null;
  }
  const activePatient = window.allPatients.find(p => p.deviceId && p.deviceActive);
  return activePatient ? activePatient.deviceId : null;
}

// Expose functions globally
window.openLinkDeviceModal = openLinkDeviceModal;
window.closeLinkDeviceModal = closeLinkDeviceModal;
window.handleLinkDevice = handleLinkDevice;
window.handleDeviceAction = handleDeviceAction;
window.getActiveDeviceId = getActiveDeviceId;
