import * as dotenv from 'dotenv';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Load environment variables
dotenv.config();

export const config = {
  port: parseInt(process.env.PORT || '3000', 10),
  nodeEnv: process.env.NODE_ENV || 'development',

  // BFG and tool paths
  bfgBinary: process.env.BFG_BINARY || '/home/arya/src/bfg/build/bfg',
  proto2gdsBinary: process.env.PROTO2GDS_BINARY || '/home/arya/src/Layout21/target/debug/proto2gds',

  // Technology files
  technologyPb: process.env.TECHNOLOGY_PB || '/home/arya/src/bfg/sky130.technology.pb',
  primitivesPb: process.env.PRIMITIVES_PB || '/home/arya/src/bfg/sky130.primitives.pb',
  sky130hdPb: process.env.SKY130HD_PB || '/home/arya/src/bfg/sky130hd.pb',

  // Temporary files
  tmpDir: process.env.TMP_DIR || path.join(__dirname, '../../tmp'),
  sessionCleanupInterval: parseInt(process.env.SESSION_CLEANUP_INTERVAL_MS || '3600000', 10),
  sessionMaxAge: parseInt(process.env.SESSION_MAX_AGE_MS || '3600000', 10),
  maxConcurrentSessions: parseInt(process.env.MAX_CONCURRENT_SESSIONS || '100', 10),
};
