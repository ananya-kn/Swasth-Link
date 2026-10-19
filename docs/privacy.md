# Privacy and data handling (prototype)

- No patient names anywhere. A household/patient code identifies a person to the ASHA only.
- Alerts (SMS/webhook) carry case ID, vitals, symptoms and a map link; no name, no phone number of the patient.
- Server: API keys per role (demo keys must be replaced via env), HTTPS only in any real deployment (put it behind Caddy/Nginx), keep `server/data/` off public storage.
- Retention: decide and document (for example 12 months), and provide delete-on-request.
- Law and policy to design against: India's Digital Personal Data Protection Act 2023 (consent, purpose limitation, minimisation), and ABDM consent flows if records are linked to ABHA. This prototype is not compliant by itself; the doc is a checklist.
- Regulatory note: a device that informs clinical decisions can fall under the CDSCO Medical Devices Rules 2017. Position it as a screening/triage aid and take advice before any field deployment.
