// API client for communicating with the backend

import axios from 'axios';

export interface GenerateRequest {
  generator: string;
  parameters: Record<string, unknown>;
}

export interface GenerateResponse {
  status: 'success' | 'error';
  data?: {
    svg: string;
    libraryProto: string;
    packageProto: string;
  };
  message?: string;
  details?: string;
}

const API_BASE_URL = import.meta.env.VITE_API_URL || '/api';

export const apiClient = {
  async generate(request: GenerateRequest): Promise<GenerateResponse> {
    const response = await axios.post<GenerateResponse>(
      `${API_BASE_URL}/generate`,
      request
    );
    return response.data;
  },

  async health(): Promise<{ status: string; timestamp: string }> {
    const response = await axios.get(`${API_BASE_URL}/health`);
    return response.data;
  },
};
