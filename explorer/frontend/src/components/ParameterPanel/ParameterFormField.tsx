import React from 'react';
import { FieldSchema } from '../../schemas/parameterSchemas';
import './ParameterFormField.css';

interface ParameterFormFieldProps {
  field: FieldSchema;
  value: unknown;
  onChange: (value: unknown) => void;
  onComplete?: () => void;
}

const ParameterFormField: React.FC<ParameterFormFieldProps> = ({
  field,
  value,
  onChange,
  onComplete,
}) => {
  const renderField = () => {
    switch (field.type) {
      case 'number':
        return (
          <div className="field-input-wrapper">
            <input
              type="number"
              value={value as number ?? ''}
              onChange={(e) => {
                const val = e.target.value === '' ? undefined : Number(e.target.value);
                onChange(val);
              }}
              onBlur={() => onComplete?.()}
              min={field.min}
              max={field.max}
              step={field.step ?? 1}
              placeholder={field.defaultValue?.toString()}
            />
            {field.unit && <span className="field-unit">{field.unit}</span>}
          </div>
        );

      case 'boolean':
        return (
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={value as boolean ?? field.defaultValue ?? false}
              onChange={(e) => {
                onChange(e.target.checked);
                onComplete?.();
              }}
            />
            <span className="checkbox-text">
              {value ? 'Enabled' : 'Disabled'}
            </span>
          </label>
        );

      case 'enum':
        return (
          <select
            value={value as string ?? ''}
            onChange={(e) => {
              onChange(e.target.value || undefined);
              onComplete?.();
            }}
          >
            <option value="">-- Select --</option>
            {field.enumValues?.map((enumValue) => (
              <option key={enumValue} value={enumValue}>
                {enumValue}
              </option>
            ))}
          </select>
        );

      case 'string':
        return (
          <input
            type="text"
            value={value as string ?? ''}
            onChange={(e) => onChange(e.target.value || undefined)}
            onBlur={() => onComplete?.()}
            placeholder={field.defaultValue as string}
          />
        );

      case 'array':
        return renderArrayField();

      default:
        return <div className="field-unsupported">Unsupported field type</div>;
    }
  };

  const renderArrayField = () => {
    const arrayValue = (value as Array<unknown>) ?? [];

    if (field.itemType === 'object' && field.itemFields) {
      // For Sky130TransmissionGateStack sequences
      return (
        <div className="array-field">
          {arrayValue.map((item, index) => (
            <div key={index} className="array-item">
              <div className="array-item-header">
                <span className="array-item-label">Sequence {index + 1}</span>
                <button
                  type="button"
                  onClick={() => {
                    const newArray = [...arrayValue];
                    newArray.splice(index, 1);
                    onChange(newArray);
                    onComplete?.();
                  }}
                  className="array-remove-btn"
                >
                  Remove
                </button>
              </div>
              {field.itemFields.map((itemField) => {
                const itemValue = (item as Record<string, unknown>)?.[itemField.name];

                // Special handling for nets field (comma-separated string)
                if (itemField.name === 'nets') {
                  const netsString = Array.isArray(itemValue)
                    ? itemValue.join(',')
                    : (itemValue as string ?? '');

                  return (
                    <div key={itemField.name} className="array-item-field">
                      <label>{itemField.label}</label>
                      <input
                        type="text"
                        value={netsString}
                        onChange={(e) => {
                          const newArray = [...arrayValue];
                          const nets = e.target.value.split(',').map(s => s.trim()).filter(s => s);
                          newArray[index] = {
                            ...(item as Record<string, unknown>),
                            nets,
                          };
                          onChange(newArray);
                        }}
                        onBlur={() => onComplete?.()}
                        placeholder="e.g., A,B,C"
                      />
                      <small className="field-help">{itemField.description}</small>
                    </div>
                  );
                }

                return null;
              })}
            </div>
          ))}
          <button
            type="button"
            onClick={() => {
              onChange([...arrayValue, { nets: [] }]);
              onComplete?.();
            }}
            className="array-add-btn"
          >
            + Add Sequence
          </button>
        </div>
      );
    }

    return <div className="field-unsupported">Unsupported array type</div>;
  };

  return (
    <div className="parameter-field">
      <label className="field-label">
        {field.label}
        {field.description && (
          <span className="field-help-icon" title={field.description}>
            ?
          </span>
        )}
      </label>
      {renderField()}
      {field.description && field.type !== 'boolean' && (
        <small className="field-help">{field.description}</small>
      )}
    </div>
  );
};

export default ParameterFormField;
