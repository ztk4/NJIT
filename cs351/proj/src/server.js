// Main executable for the server.
// Invoking this script under node starts the web server for the app.
// Allows the web port to be specified via the environment variable PORT.
// Allows environment to be specified via the environment variable NODE_ENV.
//   - NODE_ENV should be either "production" or "development".
//
// The server will be hosted over TLSv1.2 using a self-signed certificate.
// All routing will be handled by app.js

// Filesystem lib.
import fs from 'fs';
// HTTP server over SSL/TLS.
import https from 'https';
// Library for manipulating file paths.
import path from 'path';

// Include our app from app.js (contains all server-side logic).
// This app will handle all requests to the https server below.
//import app from './app';
import app from './app';

// HTTPS server options.
const server_opts = {
  // TLS Secure Context Options.
  // We support only auth certificates signed by our app directly.
  // This is a simple and suffice protocol for this self-contained project.
  ca: [ fs.readFileSync(path.join(__dirname, '../.certs/cert.pem')) ],
  // This server's key and certificate files.
  key: fs.readFileSync(path.join(__dirname, '../.certs/key.pem')),
  cert: fs.readFileSync(path.join(__dirname, '../.certs/cert.pem')),
  // TODO: Explore encryption options (DHE parameters and curves).
  // Force the use of TLSv1.2. Since we control all clients this is doable.
  secureProtocol: 'TLSv1_2_method',
};

// Port to host server on, defaults to 8080.
const port = parseInt(process.env.PORT, 10) || 8080;

// Create an HTTPS server over TLS as specified above.
let server = https.createServer(server_opts, app)
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
    });
  } else {
    // Exit right away if server doesn't exist or close is unavailable.
    process.exit(0);
  }
}
// NOTE: we leave the default behavior for SIGHUP.
process.on('SIGTERM', on_term).on('SIGINT', on_term);
