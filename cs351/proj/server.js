// Main executable for the server.
// Invoking this script under node starts the web server for the app.
// Allows the web port to be specified via the environment variable PORT.
//
// The server will be hosted over TLSv1.2 using a self-signed certificate.
// All routing will be handled by app.js


// Filesystem lib.
const fs = require('fs');
// HTTP server over SSL/TLS.
const https = require('https');

// Include our app from app.js (contains all server-side logic).
// This app will handle all requests to the https server below.
const app = require('./app');

// HTTPS server options.
/* Removed for testing basics of the server.
const server_opts = {
  // TLS Secure Context Options.
  // We support only certificates signed by our app directly.
  // This is a simple and suffice protocol for this self-contained project.
  ca: [ fs.readFileSync('TODO: CERT.pem') ],
  // This server's key and certificate files.
  key: fs.readFileSync('TODO: KEY.pem'),
  cert: fs.readFileSync('TODO: CERT.pem'),
  // TODO: Explore encryption options (DHE parameters and curves).
  // Force the use of TLSv1.2. Since we control all clients this is doable.
  secureProtocol: 'TLSv1_2_method',
  // TLS Server Options.
  // Request a certificate from clients.
  requestCert: true,
  // We can't just immediately reject unauthorized clients because they may just
  // be a new user. Instead, this logic will be handled via middleware.
  rejectUnauthorized: false,
};
*/
const server_opts = {};

// Port to host server on, defaults to 8080.
const port = parseInt(process.env.PORT, 10) || 8080;

// Create an HTTPS server over TLS as specified above.
server = https.createServer(server_opts, app)
    .on('listening', function () {
      console.log(`The HTTPS server is listening on port ${port} since ${new Date().toLocaleString()} ET`);
    })
    .on('error', function (err) {
      // Log the error.
      // TODO: Consider gracefully handling certain error types.
      console.err('Fatal Error Ocurred:');
      console.err(err);

      console.err('Terminating the server...');
      server.close(function() {
        // Error exit code since server died from error.
        process.exit(1);
      });
    })
    .on('close', function () {
      console.log(`The HTTPS server was terminated on ${new Date().toLocaleString()} ET`);
    })
    .listen(port);

// Listen for OS signals to safely terminate server.
function on_term() {
  // Close the server as a response to a terminate signal.
  if (server && server.close) {
    server.close(function() {
      // Exit with code 0 after close finishes.
      process.exit(0);
    }
  } else {
    // Exit right away if server doesn't exit or close is unavailable.
    process.exit(0);
  }
}
process.on('SIGTERM', on_term).on('SIGINT', on_term);
// NOTE: we leave the default behavior for SIGHUP.
